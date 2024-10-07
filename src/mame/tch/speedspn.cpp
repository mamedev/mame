// license:BSD-3-Clause
// copyright-holders: David Haywood, Farfetch'd

/*** DRIVER INFO & NOTES ******************************************************
 Speed Spin (c)1994 TCH
  driver by David Haywood & Farfetch'd

Notes:
- To enter "easy" service mode, keep some input pressed during boot,
  e.g. BUTTON 1.

TODO:
- Unknown Port Writes:
  cpu #0 (PC=00000D88): unmapped port byte write to 00000001 = 02
  cpu #0 (PC=00006974): unmapped port byte write to 00000010 = 10
  cpu #0 (PC=00006902): unmapped port byte write to 00000010 = 20
  etc.
- Writes to ROM regions
  cpu #0 (PC=00001119): byte write to ROM 0000C8B9 = 01
  cpu #0 (PC=00001119): byte write to ROM 0000C899 = 01
  etc.

******************************************************************************/

/*** README INFO **************************************************************

ROMSET: speedspn

Speed Spin
1994, TCH

PCB No: PR02/2
CPU   : Z80 (6Mhz)
SOUND : Z80 (6Mhz), OKI M6295
RAM   : 62256 (x1), 6264 (x1), 6116 (x6)
XTAL  : 12.000MHz (near Z80's), 10.000MHz (near PLCC84)
DIP   : 8 position (x2)
OTHER : TPC1020AFN-084C (PLCC84, Gfx controller)

ROMs          Type    Used            C'sum
-------------------------------------------
TCH-SS1.u78   27C040  Main Program    C246h
TCH-SS2.u96   27C512  Sound Program   5D04h
TCH-SS3.u95   27C040  Oki Samples     7340h
TCH-SS4.u70   27C010  \               ECD8h
TCH-SS5.u69     "     |               9E7Bh
TCH-SS6.u60     "     |               E844h
TCH-SS7.u59     "     |  Gfx          3DDah
TCH-SS8.u39     "     |               A9B5h
TCH-SS9.u34     "     /               AB2Bh


******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_PRGBANK    (1U << 1)
#define LOG_VIDRAMBANK (1U << 2)
#define LOG_VIDEOFLAGS (1U << 3)

//#define VERBOSE (LOG_GENERAL | LOG_PRGBANK | LOG_VIDRAMBANK | LOG_VIDEOFLAGS)

#include "logmacro.h"

#define LOGPRGBANK(...)    LOGMASKED(LOG_PRGBANK,    __VA_ARGS__)
#define LOGVIDRAMBANK(...) LOGMASKED(LOG_VIDRAMBANK, __VA_ARGS__)
#define LOGVIDEOFLAGS(...) LOGMASKED(LOG_VIDEOFLAGS, __VA_ARGS__)


namespace {

class speedspn_state : public driver_device
{
public:
	speedspn_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_prgbank(*this, "prgbank"),
		m_okibank(*this, "okibank"),
		m_videoview(*this, "videoview"),
		m_attram(*this, "attram"),
		m_spriteram(*this, "spriteram"),
		m_tileram(*this, "tileram")
	{ }

	void speedspn(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_memory_bank m_prgbank;
	required_memory_bank m_okibank;
	memory_view m_videoview;
	required_shared_ptr<uint8_t> m_attram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_tileram;

	tilemap_t *m_tilemap = nullptr;
	bool m_display_disable = false;
	uint8_t irq_ack_r();
	void rombank_w(uint8_t data);
	void tileram_w(offs_t offset, uint8_t data);
	void attram_w(offs_t offset, uint8_t data);
	void vidram_bank_w(uint8_t data);
	void display_disable_w(uint8_t data);
	void okibank_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_io_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
	void main_program_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

TILE_GET_INFO_MEMBER(speedspn_state::get_tile_info)
{
	int const code = m_tileram[tile_index * 2 + 1] | (m_tileram[tile_index * 2] << 8);
	int const attr = m_attram[tile_index ^ 0x400];

	tileinfo.set(0, code, attr & 0x3f, (attr & 0x80) ? TILE_FLIPX : 0);
}

void speedspn_state::video_start()
{
	m_display_disable = false;
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(speedspn_state::get_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 64, 32);

	save_item(NAME(m_display_disable));
}

void speedspn_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[offset] = data;

	m_tilemap->mark_tile_dirty(offset / 2);
}

void speedspn_state::attram_w(offs_t offset, uint8_t data)
{
	m_attram[offset] = data;

	m_tilemap->mark_tile_dirty(offset ^ 0x400);
}

void speedspn_state::vidram_bank_w(uint8_t data)
{
	LOGVIDRAMBANK("VidRam bank: %04x\n", data);
	m_videoview.select(data & 1);
}

void speedspn_state::display_disable_w(uint8_t data)
{
	LOGVIDEOFLAGS("Global display: %u\n", data);
	m_display_disable = data & 1;
}


void speedspn_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(1);
	uint8_t *source = &m_spriteram[0];
	uint8_t *finish = source + 0x1000;

	while (source < finish)
	{
		int xpos = source[0];
		int tileno = source[1];
		int const attr = source[2];
		int const ypos = source[3];

		if (!attr && xpos) break; // end of sprite list marker?

		if (attr & 0x10) xpos += 0x100;

		xpos = 0x1f8 - xpos;
		tileno += ((attr & 0xe0) >> 5) * 0x100;
		int const color = attr & 0x0f;

		gfx->transpen(bitmap, cliprect,
				tileno,
				color,
				0, 0,
				xpos, ypos, 15);

		source += 4;
	}
}


uint32_t speedspn_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_display_disable)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	m_tilemap->set_scrollx(0, 0x100); // verify
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


uint8_t speedspn_state::irq_ack_r()
{
	// I think this simply acknowledges the IRQ #0, it's read within the handler and the
	// value is discarded

	return 0;
}

void speedspn_state::rombank_w(uint8_t data)
{
	if (data > 8)
	{
		LOGPRGBANK("Unmapped bank write %02x", data);
		data = 0;
	}
	m_prgbank->set_entry(data);
}

/*** SOUND RELATED ***********************************************************/

void speedspn_state::okibank_w(uint8_t data)
{
	m_okibank->set_entry(data & 3);
}

/*** MEMORY MAPS *************************************************************/

// main CPU

void speedspn_state::main_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette"); // RAM COLOUR
	map(0x8800, 0x8fff).ram().w(FUNC(speedspn_state::attram_w)).share(m_attram);
	map(0x9000, 0x9fff).view(m_videoview); // RAM FIX / RAM OBJECTS (selected by bit 0 of port 17)
	m_videoview[0](0x9000, 0x9fff).ram().w(FUNC(speedspn_state::tileram_w)).share(m_tileram);
	m_videoview[1](0x9000, 0x9fff).ram().share(m_spriteram);
	map(0xa000, 0xa7ff).ram();
	map(0xa800, 0xafff).ram();
	map(0xb000, 0xbfff).ram(); // RAM PROGRAM
	map(0xc000, 0xffff).bankr(m_prgbank);
}

void speedspn_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x07, 0x07).w(FUNC(speedspn_state::display_disable_w));
	map(0x10, 0x10).portr("SYSTEM");
	map(0x11, 0x11).portr("P1");
	map(0x12, 0x12).portr("P2").w(FUNC(speedspn_state::rombank_w));
	map(0x13, 0x13).portr("DSW1").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x14, 0x14).portr("DSW2");
	map(0x16, 0x16).r(FUNC(speedspn_state::irq_ack_r)); // @@@ could be watchdog, value is discarded
	map(0x17, 0x17).w(FUNC(speedspn_state::vidram_bank_w));
}

// sound CPU

void speedspn_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9000).w(FUNC(speedspn_state::okibank_w));
	map(0x9800, 0x9800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r("soundlatch", FUNC(generic_latch_8_device::read));
}

void speedspn_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr(m_okibank);
}

/*** INPUT PORT **************************************************************/

static INPUT_PORTS_START( speedspn )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x05, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:4,3,2,1")
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x50, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "World Cup" )         PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Backhand" )          PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, "Automatic" )
	PORT_DIPSETTING(    0x00, "Manual" )
	PORT_DIPNAME( 0x0c, 0x0c, "Points to Win" )     PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x0c, "11 Points and 1 Advantage" )
	PORT_DIPSETTING(    0x08, "11 Points and 2 Advantage" )
	PORT_DIPSETTING(    0x04, "21 Points and 1 Advantage" )
	PORT_DIPSETTING(    0x00, "21 Points and 2 Advantage" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x080, IP_ACTIVE_LOW, "SW2:1" )
INPUT_PORTS_END

/*** GFX DECODE **************************************************************/

static const gfx_layout speedspn_charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(2,4), RGN_FRAC(3,4), RGN_FRAC(0,4), RGN_FRAC(1,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8
};

static const gfx_layout speedspn_spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 4, 0, RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0 },
	{ 16*16+11, 16*16+10, 16*16+9, 16*16+8, 16*16+3, 16*16+2, 16*16+1, 16*16+0,
			11,       10,       9,       8,       3,       2,       1,       0  },
	{ 8*16+7*16, 8*16+6*16, 8*16+5*16, 8*16+4*16, 8*16+3*16, 8*16+2*16, 8*16+1*16, 8*16+0*16,
			7*16,      6*16,      5*16,      4*16,      3*16,      2*16,      1*16,      0*16  },
	32*16
};


static GFXDECODE_START( gfx_speedspn )
	GFXDECODE_ENTRY( "tiles",   0, speedspn_charlayout,   0x000, 0x40 )
	GFXDECODE_ENTRY( "sprites", 0, speedspn_spritelayout, 0x000, 0x40 )
GFXDECODE_END

/*** MACHINE DRIVER **********************************************************/

void speedspn_state::machine_start()
{
	// is this weird banking some form of protection? TODO: 0x20000 isn't empty but isn't banked anywhere?
	uint8_t *rom = memregion("maincpu")->base();
	m_prgbank->configure_entry(0, &rom[0x28000]);
	m_prgbank->configure_entry(1, &rom[0x14000]);
	m_prgbank->configure_entry(2, &rom[0x1c000]);
	m_prgbank->configure_entry(3, &rom[0x54000]);
	m_prgbank->configure_entry(4, &rom[0x48000]);
	m_prgbank->configure_entry(5, &rom[0x3c000]);
	m_prgbank->configure_entry(6, &rom[0x18000]);
	m_prgbank->configure_entry(7, &rom[0x4c000]);
	m_prgbank->configure_entry(8, &rom[0x50000]);
	m_prgbank->set_entry(0);

	m_okibank->configure_entries(0, 4, memregion("oki")->base(), 0x20000);
	m_okibank->set_entry(0);
}


void speedspn_state::speedspn(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 12_MHz_XTAL / 2); // 6 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &speedspn_state::main_program_map);
	m_maincpu->set_addrmap(AS_IO, &speedspn_state::main_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(speedspn_state::irq0_line_hold));

	Z80(config, m_audiocpu, 12_MHz_XTAL / 2); // 6 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &speedspn_state::sound_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8*8, 56*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(speedspn_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_speedspn);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 0x400);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, 0);

	okim6295_device &oki(OKIM6295(config, "oki", 1'122'000, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
	oki.set_addrmap(0, &speedspn_state::oki_map);
}

/*** ROM LOADING *************************************************************/

ROM_START( speedspn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tch-ss1.u78", 0x00000, 0x80000, CRC(41b6b45b) SHA1(d969119959db4cc3be50f188bfa41e4b4896eaca) ) // banked

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "tch-ss2.u96", 0x00000, 0x10000, CRC(4611fd0c) SHA1(b49ad6a8be6ccfef0b2ed187fb3b008fb7eeb2b5) ) // FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x80000, "oki", 0 )  // samples, banked
	ROM_LOAD( "tch-ss3.u95", 0x00000, 0x80000, CRC(1c9deb5e) SHA1(89f01a8e8bdb0eee47e9195b312d2e65d41d3548) )

	ROM_REGION( 0x80000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "tch-ss4.u70", 0x00000, 0x20000, CRC(41517859) SHA1(3c5102e41c5a70e02ed88ea43ca63edf13f4c1b9) )
	ROM_LOAD( "tch-ss5.u69", 0x20000, 0x20000, CRC(832b2f34) SHA1(7a3060869a9698c9ed4187b239a70e273de64e3c) )
	ROM_LOAD( "tch-ss6.u60", 0x40000, 0x20000, CRC(f1fd7289) SHA1(8950ef58efdffc45d68152257ca36aedf5ddf677) )
	ROM_LOAD( "tch-ss7.u59", 0x60000, 0x20000, CRC(c4958543) SHA1(c959b440801707c30a8968a1f44abe5442d03eff) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "tch-ss8.u39", 0x00000, 0x20000, CRC(2f27b16d) SHA1(7cc017fa08573f8a9d94d017abb987f8288bcd29) )
	ROM_LOAD( "tch-ss9.u34", 0x20000, 0x20000, CRC(c372f8ec) SHA1(514bef0859c0adfd9cdd22864230fc83e9b1962d) )
ROM_END

} // anonymous namespace


/*** GAME DRIVERS ************************************************************/

GAME( 1994, speedspn, 0, speedspn, speedspn, speedspn_state, empty_init, ROT180, "TCH", "Speed Spin", MACHINE_SUPPORTS_SAVE )
