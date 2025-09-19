// license:BSD-3-Clause
// copyright-holders:

/*
Excellent System's ES-9501 PCB
Also seen as ES9901 with same components / locations

Main components:
TMP68HC000-P16 CPU
2x HM6265LK-70 RAM (near CPU)
28.6363 MHz XTAL (near CPU)
ES-9409 custom (GFX, same as dblcrown.cpp)
6x N341256P-15 RAM (near custom)
IS61C64AH-20N RAM (near custom)
2x N341256P-15 RAM (near custom)
YMZ280B-F sound chip (no XTAL)
YMZ284-D SSGL (in audio amp area)
MAX693ACPE watchdog
93C56 EEPROM
bank of 8 DIP switches
battery (near CPU)

Undumped games known to run on this PCB:
* Multi Spin

TODO
- EEPROM write doesn't work;
- layout for lamps;
- sprite colors aren't always correct (i.e. specd9 title screen). Is palette banked, too?;
- verify sprites / tilemaps priorities;
- flip screen support;
- only a small part of the videoregs are (perhaps) understood;
- d9flower needs correct EEPROM;
- d9flower doesn't update palette after boot and doesn't accept controls (IRQ problem?)
- device-ify ES-9409 and share with excellent/dblcrown.cpp.
*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/bankdev.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/ymz280b.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_EEPROM     (1U << 1)
#define LOG_LAMPS      (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_EEPROM | LOG_LAMPS)

#include "logmacro.h"

#define LOGEEPROM(...)     LOGMASKED(LOG_EEPROM,     __VA_ARGS__)
#define LOGLAMPS(...)      LOGMASKED(LOG_LAMPS,      __VA_ARGS__)


namespace {

class es9501_state : public driver_device
{
public:
	es9501_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_watchdog(*this, "watchdog"),
		m_eeprom(*this, "eeprom"),
		m_hopper(*this, "hopper"),
		m_vram_bank(*this, "vram_bank%u", 0U),
		m_vram(*this, "vram"),
		m_spriteram(*this, "spriteram", 0x2000U, ENDIANNESS_BIG),
		m_videoregs(*this, "videoregs", 0x20U, ENDIANNESS_BIG),
		m_pal_ram(*this, "palram", 0x200U, ENDIANNESS_BIG)
	{ }

	void es9501(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<hopper_device> m_hopper;
	required_device_array<address_map_bank_device, 2> m_vram_bank;

	required_shared_ptr<u8> m_vram;
	memory_share_creator<u8> m_spriteram;
	memory_share_creator<u8> m_videoregs;
	memory_share_creator<u8> m_pal_ram;

	u8 m_irq_source = 0;
	u8 m_irq_mask = 0;
	u8 m_vram_bank_entry[2] {};
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_md_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	template <u8 Which> TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void vram_w(offs_t offset, u8 data);
	u8 spriteram_r(offs_t offset) { return m_spriteram[offset]; }
	void spriteram_w(offs_t offset, u8 data) { m_spriteram[offset] = data; }
	u8 videoregs_r(offs_t offset) { return m_videoregs[offset]; }
	void videoregs_w(offs_t offset, u8 data) { m_videoregs[offset] = data; }
	void palette_w(offs_t offset, u8 data);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	template <u8 Which> void vram_bank_w(u8 data);

	void watchdog_eeprom_w(u8 data);
	void counters_w(u8 data);
	void hopper_w(u8 data);
	void lamps_w(u8 data);

	void program_map(address_map &map) ATTR_COLD;
	void vram_map(address_map &map) ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);
};


void es9501_state::video_start()
{
	// TODO: hardware probably supports 2 more layers at 0xa800 and 0xac00, unused from currently dumped games

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(es9501_state::get_bg_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 16);
	m_bg_tilemap->set_user_data(&m_vram[0xa400]);

	m_md_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(es9501_state::get_bg_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 16);
	m_md_tilemap->set_user_data(&m_vram[0xa000]);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(es9501_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap->set_user_data(&m_vram[0xb000]);

	m_bg_tilemap->set_transparent_pen(0);
	m_md_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(es9501_state::get_bg_tile_info)
{
	u8 const *rambase = (const u8 *)tilemap.user_data();
	u16 const code = ((rambase[tile_index * 2 + 1] | (rambase[tile_index * 2 + 0] << 8)) & 0xfff) | (BIT(m_videoregs[Which ^ 3], 2, 3) << 12);
	u8 const color = (rambase[tile_index * 2 + 0] >> 4);

	tileinfo.set(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(es9501_state::get_fg_tile_info)
{
	u8 const *rambase = (const u8 *)tilemap.user_data();
	u16 const code = (rambase[tile_index * 2 + 1] | (rambase[tile_index * 2 + 0] << 8)) & 0x7ff;
	u8 const color = (rambase[tile_index * 2 + 0] >> 4);

	tileinfo.set(1, code, color, 0);
}

void es9501_state::vram_w(offs_t offset, u8 data)
{
	m_vram[offset] = data;

	if ((offset & 0xfc00) == 0xa000)
		m_md_tilemap->mark_tile_dirty((offset & 0x3ff) >> 1);

	if ((offset & 0xfc00) == 0xa400)
		m_bg_tilemap->mark_tile_dirty((offset & 0x3ff) >> 1);

	if ((offset & 0xf000) == 0xb000)
		m_fg_tilemap->mark_tile_dirty((offset & 0xfff) >> 1);

	m_gfxdecode->gfx(1)->mark_dirty(offset);
}

void es9501_state::palette_w(offs_t offset, u8 data)
{
	m_pal_ram[offset] = data;
	offset >>= 1;
	int const datax = m_pal_ram[offset * 2] + 256 * m_pal_ram[offset * 2 + 1];

	int const r = ((datax) & 0x0f00) >> 8;
	int const g = ((datax) & 0xf000) >> 12;
	int const b = ((datax) & 0x000f) >> 0;
	// TODO: remaining bits

	m_palette->set_pen_color(offset, pal4bit(r), pal4bit(g), pal4bit(b));
}

void es9501_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_spriteram.bytes() - 8; offs >= 0; offs -= 8)
	{
		if (!BIT(m_spriteram[offs + 2], 7))
			continue;

		int sprite = m_spriteram[offs + 1] | (m_spriteram[offs] << 8);
		int const x = m_spriteram[offs + 5] | (m_spriteram[offs + 4] << 8);
		int const y = m_spriteram[offs + 7] | (m_spriteram[offs + 6] << 8);
		int const flipx = 0; // TODO
		int const flipy = 0; // TODO
		// TODO: wrong colors on title screen
		int const color = (m_spriteram[offs + 3]) & 0xf;

		// TODO: needs prio_transpen?
		// TODO: sketchy alignment
		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect, sprite, color, flipx, flipy, x - 38, y - 17, 0);
	}
}

u32 es9501_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// flip_screen_set(BIT(m_videoregs[5], 4));

	bitmap.fill(0, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	if (!BIT(m_videoregs[5], 3)) draw_sprites(bitmap, cliprect);
	m_md_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	if (BIT(m_videoregs[5], 3)) draw_sprites(bitmap, cliprect);

	return 0;
}


void es9501_state::machine_start()
{
	save_item(NAME(m_irq_source));
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_vram_bank_entry));
}

template <u8 Which>
void es9501_state::vram_bank_w(u8 data)
{
	m_vram_bank_entry[Which] = data;
	m_vram_bank[Which]->set_bank(m_vram_bank_entry[Which]);
}

void es9501_state::watchdog_eeprom_w(u8 data)
{
	if (BIT(data, 0))
		m_watchdog->watchdog_reset();

	if (data & 0x2e)
		LOGEEPROM("unknown watchdog_eeprom_w bits used: %02x\n", data & 0x2e);

	m_eeprom->di_write(BIT(data, 7));
	m_eeprom->clk_write(BIT(data, 6));
	m_eeprom->cs_write(BIT(data, 4));
}

void es9501_state::counters_w(u8 data)
{
	for (int i = 0; i < 0x06; i++)
		machine().bookkeeping().coin_counter_w(i, BIT(data, i)); // coin 1-3, hopper out, key in, key out

	if (BIT(data, 6))
		logerror("unknown counters_w bit 6 used: %02x\n", data);

	// bit 7 is always on, but is off in the key test screen.
	// it appears to be coin lockout, but then it's turned on when you test the coin inputs?
	// machine().bookkeeping().coin_lockout_global_w(!BIT(data, 7));
}

void es9501_state::hopper_w(u8 data)
{
	m_hopper->motor_w(BIT(data, 7));

	if (data & 0x7f)
		logerror("unknown hopper_w bits used: %02x\n", data & 0x7f);
}

void es9501_state::lamps_w(u8 data)
{
	// probably as follows
	// bit 0: start lamp
	// bit 1: bet lamp
	// bit 2: take or double up (both bits are always on together)
	// bit 3: take or double up (both bits are always on together)
	// bit 4: big or small (both bits are always on together)
	// bit 5: big or small (both bits are always on together)
	// bit 6: ? on after first coin insertion
	// bit 7: always 0?

	if (data & 0xc0)
		LOGLAMPS("unknown lamps_w bits used: %02x\n", data & 0xc0);
}


void es9501_state::program_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x07ffff).rom();
	map(0x3fc000, 0x3fffff).ram().share("nvram");
	map(0x400000, 0x401fff).m(m_vram_bank[0], FUNC(address_map_bank_device::amap8)).umask16(0xff00);
	map(0x402000, 0x403fff).m(m_vram_bank[1], FUNC(address_map_bank_device::amap8)).umask16(0xff00);
	map(0x404000, 0x405fff).r(FUNC(es9501_state::spriteram_r)).w(FUNC(es9501_state::spriteram_w)).umask16(0xff00);
	map(0x406000, 0x4063ff).lr8(NAME([this] (offs_t offset) { return m_pal_ram[offset]; })).w(FUNC(es9501_state::palette_w)).umask16(0xff00);
	map(0x406400, 0x406fff).ram();
	map(0x407c00, 0x407c3f).r(FUNC(es9501_state::videoregs_r)).w(FUNC(es9501_state::videoregs_w)).umask16(0xff00);
	map(0x407c40, 0x407cff).ram();
	map(0x407e00, 0x407e00).lr8(NAME([this] () -> u8 { return m_vram_bank_entry[1]; })).w(FUNC(es9501_state::vram_bank_w<1>));
	map(0x407e02, 0x407e02).lr8(NAME([this] () -> u8 { return m_vram_bank_entry[0]; })).w(FUNC(es9501_state::vram_bank_w<0>));
	map(0x407e04, 0x407e07).ram(); // ??
	map(0x407e08, 0x407e08).lrw8(
		NAME([this] () {
			return m_irq_mask;
		}),
		NAME([this] (u8 data) {
			m_irq_mask = data;
		})
	);
	map(0x407e0a, 0x407e0a).lrw8(
		NAME([this] () { return m_irq_source; }),
		NAME([this] (u8 data) { m_irq_source &= data; })
	);
	map(0x600000, 0x600001).portr("IN0").nopw();
	map(0x600000, 0x600000).w(FUNC(es9501_state::lamps_w));
	map(0x600002, 0x600003).portr("IN1");
	map(0x600002, 0x600002).w(FUNC(es9501_state::hopper_w));
	map(0x600004, 0x600005).portr("IN2");
	map(0x600004, 0x600004).w(FUNC(es9501_state::counters_w));
	map(0x600006, 0x600007).portr("DSW");
	map(0x600008, 0x600009).portr("EEPROM_IN");
	map(0x600008, 0x600008).w(FUNC(es9501_state::watchdog_eeprom_w));
	map(0x700000, 0x700003).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0xff00);
	map(0x700004, 0x700007).w("ymz284", FUNC(ymz284_device::address_data_w)).umask16(0xff00);
}

void es9501_state::vram_map(address_map &map)
{
	map(0x0000, 0xffff).ram().w(FUNC(es9501_state::vram_w)).share(m_vram);
}


// inputs according to test mode
// PCB has one bank of switches but settings are done via software
static INPUT_PORTS_START( specd9 )
	PORT_START("IN0")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) // Small
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) // Big
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))

	PORT_START("IN1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("IN2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_MEMORY_RESET )

	PORT_START("DSW")
	PORT_BIT(                      0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME(          0x0100, 0x0100, "Win Rate Configuration Screen" ) PORT_DIPLOCATION( "SW1:1" )
	PORT_DIPSETTING(               0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(               0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW1:8" )

	PORT_START("EEPROM_IN")
	PORT_BIT( 0x7fff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
INPUT_PORTS_END


const gfx_layout gfx_8x8x4_packed_msb_r =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 12, 8, 4, 0, 28, 24, 20, 16 },
	{ STEP8(0,4*8) },
	8*8*4
};

static GFXDECODE_START( gfx_es9501 )
	GFXDECODE_ENTRY( "gfx", 0, gfx_16x16x4_packed_lsb, 0, 0x10 )
	GFXDECODE_RAM( "vram", 0, gfx_8x8x4_packed_msb_r, 0, 0x10 )
GFXDECODE_END

// bit 0 unused in specd9, d9flower depends on it on memory clear screen
// bit 1 looks vblank (would hang otherwise)
// bit 2 unknown (sprite DMA complete? Unset by specd9)
TIMER_DEVICE_CALLBACK_MEMBER(es9501_state::scanline_cb)
{
	int const scanline = param;

	if (scanline == 240 && BIT(m_irq_mask, 1))
	{
		m_maincpu->set_input_line(1, HOLD_LINE);
		m_irq_source |= 2;
	}

	if (scanline == 0 && BIT(m_irq_mask, 0))
	{
		m_maincpu->set_input_line(1, HOLD_LINE);
		m_irq_source |= 1;
	}
}


void es9501_state::es9501(machine_config &config)
{
	M68000(config, m_maincpu, 28.636363_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &es9501_state::program_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(es9501_state::scanline_cb), "screen", 0, 1);

	EEPROM_93C56_16BIT(config, m_eeprom);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	HOPPER(config, m_hopper, attotime::from_msec(100));

	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_msec(1000));

	for (auto bank : m_vram_bank)
		ADDRESS_MAP_BANK(config, bank).set_map(&es9501_state::vram_map).set_options(ENDIANNESS_LITTLE, 8, 16, 0x1000);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(es9501_state::screen_update));
	m_screen->set_size(64*8, 262);
	m_screen->set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_es9501);

	PALETTE(config, m_palette).set_entries(0x100);

	SPEAKER(config, "speaker", 2).front();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 28.636363_MHz_XTAL / 2));
	ymz.add_route(0, "speaker", 1.0, 0);
	ymz.add_route(1, "speaker", 1.0, 1);

	ymz284_device & ymz284(YMZ284(config, "ymz284", 28.636363_MHz_XTAL / 8)); // divider not verified
	ymz284.add_route(ALL_OUTPUTS, "speaker", 0.5, 0);
	ymz284.add_route(ALL_OUTPUTS, "speaker", 0.5, 1);
}


ROM_START( d9flower ) // Dream 9 Flower string, but images seem more Flower 9 Dream. No mask ROMs, only EPROMs
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2.u33", 0x00000, 0x40000, CRC(a57ef10e) SHA1(89d46c80e03b21469f61ee021013e4be51ef882e) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "1.u31", 0x00001, 0x40000, CRC(fb6c1e72) SHA1(a03e9129c52c4587fb360f2f886bbd9983f49f05) ) // 1xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x280000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "3.u50", 0x000000, 0x080000, CRC(0f1f8b61) SHA1(d33d73dcdbf06a84e6be28ac1c3273dd21d0ad17) )
	ROM_LOAD( "4.u51", 0x200000, 0x080000, CRC(c2a06ed5) SHA1(ffb07982f9ad91ce28bf3eacb8deedcc957bbbc1) )

	ROM_REGION( 0x200000, "ymz", ROMREGION_ERASE00 )
	ROM_LOAD( "5.u23", 0x000000, 0x080000, CRC(b6ad2e58) SHA1(84c0cdc155f641d4e5d8ae99acbfa5b297762418) )

	ROM_REGION16_BE( 0x100, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "93c56.u12", 0x000, 0x100, NO_DUMP )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "3.u37", 0x000, 0x117, BAD_DUMP CRC(bea4cb24) SHA1(09987e6b903cc3bd202a9d933474b36bdbb99d9a) ) // not dumped for this set, but marked same
ROM_END

ROM_START( d9flowera ) // same GFX / sound ROMs as d9flower, updated program (but version string unchanged)
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2.u33", 0x00000, 0x40000, CRC(803efc2d) SHA1(1e9c6b4c8adb2c8c837fe36f8c9c7c2aa3e675d8) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "1.u31", 0x00001, 0x40000, CRC(37186597) SHA1(b67c9fd057b7cc115866f73ecfdd57bb8dd09d7b) ) // 1xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x280000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "j3.u50", 0x000000, 0x080000, CRC(0f1f8b61) SHA1(d33d73dcdbf06a84e6be28ac1c3273dd21d0ad17) )
	ROM_LOAD( "j4.u51", 0x200000, 0x080000, CRC(c2a06ed5) SHA1(ffb07982f9ad91ce28bf3eacb8deedcc957bbbc1) )

	ROM_REGION( 0x200000, "ymz", ROMREGION_ERASE00 )
	ROM_LOAD( "j5.u23", 0x000000, 0x080000, CRC(b6ad2e58) SHA1(84c0cdc155f641d4e5d8ae99acbfa5b297762418) )

	ROM_REGION16_BE( 0x100, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "93c56.u12", 0x000, 0x100, NO_DUMP )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "003.u37", 0x000, 0x117, BAD_DUMP CRC(bea4cb24) SHA1(09987e6b903cc3bd202a9d933474b36bdbb99d9a) ) // not dumped for this set, but probably same
ROM_END

ROM_START( specd9 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "3.u33", 0x00000, 0x40000, CRC(682daf75) SHA1(822b5e9443bf7e1b752da85e879a8b0994f23fdf) )
	ROM_LOAD16_BYTE( "1.u31", 0x00001, 0x40000, CRC(90b10562) SHA1(d1d3d50027e84cc028cd30d2dd74a4f6666387cb) )

	ROM_REGION( 0x280000, "gfx", 0 )
	ROM_LOAD( "t58.u50", 0x000000, 0x200000, CRC(7a572d9e) SHA1(9a1d842ac78fea6047242c405aaf81c827dc2358) ) // contains Multi Spin logo
	ROM_LOAD( "u51.u51", 0x200000, 0x080000, CRC(a213c33b) SHA1(42b4c3d3cb2db50ea0fad06509e3e73b81f3db4c) ) // this is an EPROM, contains Special Dream 9 logo

	ROM_REGION( 0x200000, "ymz", 0 )
	ROM_LOAD( "t59.u23", 0x000000, 0x200000, CRC(b11857b4) SHA1(c0a6478fd8a8ef1ed35cfbfa9fd2af44eb258725) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c56.u12", 0x000, 0x100, CRC(dba91cd8) SHA1(dfbe41e3a8d7e8ad7068d25afe10a1d93bf3cc4d) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "3.u37", 0x000, 0x117, CRC(bea4cb24) SHA1(09987e6b903cc3bd202a9d933474b36bdbb99d9a) ) // PALCE16V8H
ROM_END

ROM_START( specd9105g )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "3.u33", 0x00000, 0x40000, CRC(e4b00f37) SHA1(4c33912b7c38399ba2ca5e4dc0335458d929bd52) ) // SLDH
	ROM_LOAD16_BYTE( "2.u31", 0x00001, 0x40000, CRC(620bc09e) SHA1(fce0e9c7394aa782d0b6f1558a3b4c76c5c1e787) )

	ROM_REGION( 0x280000, "gfx", 0 )
	ROM_LOAD( "t58.u50", 0x000000, 0x200000, CRC(7a572d9e) SHA1(9a1d842ac78fea6047242c405aaf81c827dc2358) ) // contains Multi Spin logo
	ROM_LOAD( "u51.u51", 0x200000, 0x080000, CRC(a213c33b) SHA1(42b4c3d3cb2db50ea0fad06509e3e73b81f3db4c) ) // this is an EPROM, contains Special Dream 9 logo

	ROM_REGION( 0x200000, "ymz", 0 )
	ROM_LOAD( "t59.u23", 0x000000, 0x200000, CRC(b11857b4) SHA1(c0a6478fd8a8ef1ed35cfbfa9fd2af44eb258725) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c56.u12", 0x000, 0x100, CRC(dba91cd8) SHA1(dfbe41e3a8d7e8ad7068d25afe10a1d93bf3cc4d) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "3.u37", 0x000, 0x117, CRC(bea4cb24) SHA1(09987e6b903cc3bd202a9d933474b36bdbb99d9a) ) // PALCE16V8H
ROM_END

ROM_START( starball )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "3.u33", 0x00000, 0x40000, CRC(43f751ca) SHA1(b3e50f7bc939e25167da98ab51f16b23436a581e) ) // SLDH
	ROM_LOAD16_BYTE( "2.u31", 0x00001, 0x40000, CRC(d9c1088d) SHA1(d5b86d33db838418e2bb94da04c902d0059c673e) )

	ROM_REGION( 0x280000, "gfx", 0 )
	ROM_LOAD( "t58.u50", 0x000000, 0x200000, CRC(7a572d9e) SHA1(9a1d842ac78fea6047242c405aaf81c827dc2358) ) // contains Multi Spin logo
	ROM_LOAD( "1.u51",   0x200000, 0x080000, CRC(f7e97d23) SHA1(9aa86e545e9438ab693d8f9b1c137dada86be5cc) ) // this is an EPROM, contains Special Dream 9 logo

	ROM_REGION( 0x200000, "ymz", 0 )
	ROM_LOAD( "t59.u23", 0x000000, 0x200000, CRC(b11857b4) SHA1(c0a6478fd8a8ef1ed35cfbfa9fd2af44eb258725) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c56.u12", 0x000, 0x100, BAD_DUMP CRC(3d0a5809) SHA1(5d754d359c36db8a08337c61d6101050a97407e3) ) // handcrafted

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "3.u37", 0x000, 0x117, CRC(bea4cb24) SHA1(09987e6b903cc3bd202a9d933474b36bdbb99d9a) ) // PALCE16V8H
ROM_END

} // anonymous namespace


GAME( 1998, d9flower,   0,        es9501, specd9, es9501_state, empty_init, ROT0, "Cadence Technology", "Dream 9 Flower (v1.00c, set 1)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1998, d9flowera,  d9flower, es9501, specd9, es9501_state, empty_init, ROT0, "Cadence Technology", "Dream 9 Flower (v1.00c, set 2)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1997, specd9,     0,        es9501, specd9, es9501_state, empty_init, ROT0, "Excellent System",   "Special Dream 9 (v1.0.7G)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1997, specd9105g, specd9,   es9501, specd9, es9501_state, empty_init, ROT0, "Excellent System",   "Special Dream 9 (v1.0.5G)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1997, starball,   0,        es9501, specd9, es9501_state, empty_init, ROT0, "Excellent System",   "Star Ball (v1.0.0S)",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
