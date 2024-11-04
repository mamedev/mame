// license:BSD-3-Clause
// copyright-holders: Tomasz Slanina

/****************************************************************************************

Super Wing - (c) 1985 Wing (UPL?)

driver by Tomasz Slanina

probably a sequel to flipjack
Hardware a bit (interrupts, sound) similar to mouser as well

TODO:
- unused ROM 6.8s (located on the PCB near the gfx ROM 7.8p, but contains
  data (similar to the one in ROMs 4.5p and 5.5r)

  The game currently crashes after the bonus round rather than moving on to
  the next level, it writes 01 to 0xa187 which is probably ROM bank, however
  banking the ROM in there results in the game crashing anyway, and looking
  at the data I wonder if it is corrupt, there are odd patterns in FF fill
  areas.

  (to access the bonus round take out the targets on the middle-left then hit
   the ball into one of the portals at the top left)


- dump color PROM
- some unknown DSW and inputs
- hopper
- unknown writes
- measure clocks


*****************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_UNKWRITE (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_UNKWRITE)

#include "logmacro.h"

#define LOGUNKWRITE(...) LOGMASKED(LOG_UNKWRITE, __VA_ARGS__)


namespace {

class superwng_state : public driver_device
{
public:
	superwng_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram%u", 0U),
		m_colorram(*this, "colorram%u", 0U),
		m_mainbank(*this, "mainbank")
	{ }

	void superwng(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr_array<uint8_t, 2> m_videoram; // 0 BG, 1 FG
	required_shared_ptr_array<uint8_t, 2> m_colorram; // 0 BG, 1 FG

	required_memory_bank m_mainbank;

	uint8_t m_tile_bank = 0;
	uint8_t m_nmi_enable = 0;

	tilemap_t *m_tilemap[2] {};

	void nmi_enable_w(uint8_t data);
	void sound_nmi_clear_w(uint8_t data);
	template <uint8_t Which> void vram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void cram_w(offs_t offset, uint8_t data);
	void tilebank_w(uint8_t data);
	void flip_screen_w(uint8_t data);
	template <uint8_t Which> void cointcnt_w(uint8_t data);
	void hopper_w(uint8_t data);
	void unk_a187_w(uint8_t data);
	void unk_a185_w(uint8_t data);

	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_nmi_interrupt(int state);
	INTERRUPT_GEN_MEMBER(sound_nmi_assert);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

void superwng_state::unk_a187_w(uint8_t data)
{
	m_mainbank->set_entry(data & 1);
}

void superwng_state::unk_a185_w(uint8_t data)
{
	LOGUNKWRITE("unk_a185_w %02x\n", data);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(superwng_state::get_tile_info)
{
	int code = m_videoram[Which][tile_index];
	int const attr = m_colorram[Which][tile_index];

	code = (code & 0x7f) | ((attr & 0x40) << 1) | ((code & 0x80) << 1);
	code |= m_tile_bank ? 0x200 : 0;

	int const flipx = (attr & 0x80) ? TILE_FLIPX : 0;
	int const flipy = (attr & 0x80) ? TILE_FLIPY : 0;

	tileinfo.set(0, code, attr & 0xf, flipx | flipy);
}

void superwng_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(superwng_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(superwng_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_tilemap[0]->set_scrollx(0, 64);
}

uint32_t superwng_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	rectangle tmp = cliprect;

	if (flip_screen())
	{
		tmp.min_x += 32;
		m_tilemap[1]->draw(screen, bitmap, tmp, 0, 0);
	}
	else
	{
		tmp.max_x -= 32;
		m_tilemap[1]->draw(screen, bitmap, tmp, 0, 0);
	}

	//sprites
	for (int i = 0x3e; i >= 0; i -= 2)
	{
		/*      76543210
		video0: xxxxxx    code
		              x   /flip
		               x  enable?
		video1: xxxxxxxx  x
		color0: xxxxxxxx  y
		color1: xxx       unused?
		           x      ?
		            xxxx  color
		*/
		if (~m_videoram[0][i] & 1)
			continue;

		int const code = (m_videoram[0][i] >> 2) | 0x40;
		int const flip = ~m_videoram[0][i] >> 1 & 1;
		int const sx = 240 - m_videoram[0][i + 1];
		int const sy = m_colorram[0][i];
		int const color = m_colorram[0][i + 1] & 0xf;

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
						code,
						color,
						flip, flip,
						sx, sy, 0);
	}

	return 0;
}


static constexpr uint8_t colors[]= // temporary
{
	0x00, 0xc4, 0xff, 0x87, 0x00, 0xb0, 0xff, 0x2f, 0x00, 0x07, 0xff, 0xe0, 0x00, 0x86, 0xff, 0xc6,
	0x00, 0x07, 0x3f, 0xff, 0x00, 0xb0, 0x38, 0x27, 0x00, 0x20, 0xff, 0x27, 0x00, 0xa4, 0xff, 0x87,
	0x00, 0x58, 0xa8, 0x27, 0x00, 0x38, 0x3f, 0x27, 0x00, 0x80, 0xe4, 0x38, 0x00, 0x87, 0xff, 0x07,
	0x00, 0xc0, 0x07, 0x3f, 0x00, 0x1f, 0x3f, 0xff, 0x00, 0x86, 0x05, 0xff, 0x00, 0xc0, 0xe8, 0xff
};

void superwng_state::palette(palette_device &palette) const
{
	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		bit0 = BIT(colors[i], 0);
		bit1 = BIT(colors[i], 1);
		bit2 = BIT(colors[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(colors[i], 3);
		bit1 = BIT(colors[i], 4);
		bit2 = BIT(colors[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(colors[i], 6);
		bit1 = BIT(colors[i], 7);
		int const b = 0x4f * bit0 + 0xa8 * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void superwng_state::nmi_enable_w(uint8_t data)
{
	m_nmi_enable = data;
}

void superwng_state::main_nmi_interrupt(int state)
{
	if (state && BIT(m_nmi_enable, 0))
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void superwng_state::sound_nmi_clear_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

INTERRUPT_GEN_MEMBER(superwng_state::sound_nmi_assert)
{
	if (BIT(m_nmi_enable, 0))
		device.execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

template <uint8_t Which>
void superwng_state::vram_w(offs_t offset, uint8_t data)
{
	m_videoram[Which][offset] = data;
	m_tilemap[Which]->mark_tile_dirty(offset);
}

template <uint8_t Which>
void superwng_state::cram_w(offs_t offset, uint8_t data)
{
	m_colorram[Which][offset] = data;
	m_tilemap[Which]->mark_tile_dirty(offset);
}

void superwng_state::tilebank_w(uint8_t data)
{
	m_tile_bank = data;
	m_tilemap[0]->mark_all_dirty();
	m_tilemap[1]->mark_all_dirty();
}

void superwng_state::flip_screen_w(uint8_t data)
{
	flip_screen_set(~data & 0x01);
	m_tilemap[0]->mark_all_dirty();
	m_tilemap[1]->mark_all_dirty();
}

template <uint8_t Which>
void superwng_state::cointcnt_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(Which, data);
}

void superwng_state::hopper_w(uint8_t data)
{
}

void superwng_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x6fff).bankr(m_mainbank);
	map(0x7000, 0x7fff).ram();
	map(0x8000, 0x83ff).ram().w(FUNC(superwng_state::vram_w<0>)).share(m_videoram[0]);
	map(0x8400, 0x87ff).ram().w(FUNC(superwng_state::vram_w<1>)).share(m_videoram[1]);
	map(0x8800, 0x8bff).ram().w(FUNC(superwng_state::cram_w<0>)).share(m_colorram[0]);
	map(0x8c00, 0x8fff).ram().w(FUNC(superwng_state::cram_w<1>)).share(m_colorram[1]);
	map(0x9800, 0x99ff).ram();
	map(0xa000, 0xa000).portr("P1").w(FUNC(superwng_state::hopper_w));
	map(0xa080, 0xa080).portr("P2");
	map(0xa100, 0xa100).portr("DSW1").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xa180, 0xa180).portr("DSW2");
	// TODO: the following is almost certainly a LS259 or similar
	map(0xa180, 0xa180).nopw(); // watchdog? int ack?
	map(0xa181, 0xa181).w(FUNC(superwng_state::nmi_enable_w));
	map(0xa182, 0xa182).w(FUNC(superwng_state::tilebank_w));
	map(0xa183, 0xa183).w(FUNC(superwng_state::flip_screen_w));
	map(0xa184, 0xa184).w(FUNC(superwng_state::cointcnt_w<0>));
	map(0xa185, 0xa185).w(FUNC(superwng_state::unk_a185_w)); // unknown, always(?) 0
	map(0xa186, 0xa186).w(FUNC(superwng_state::cointcnt_w<1>));
	map(0xa187, 0xa187).w(FUNC(superwng_state::unk_a187_w)); // unknown, always(?) 0
}

void superwng_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).ram();
	map(0x3000, 0x3000).w(FUNC(superwng_state::sound_nmi_clear_w));
	map(0x4000, 0x4000).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x5000, 0x5000).w("ay1", FUNC(ay8910_device::address_w));
	map(0x6000, 0x6000).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x7000, 0x7000).w("ay2", FUNC(ay8910_device::address_w));
}

static INPUT_PORTS_START( superwng )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Shoot")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Right Flipper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Left Flipper")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL PORT_NAME("P2 Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_NAME("P2 Shoot")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Right Flipper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Left Flipper")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Tilt")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown )) // hopper related, writes 0 to 0xa000 every frame if it is set
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown )) // hopper related, if 0x20 is set, and this is set, it will lock up with HOPPER EMPTY
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0,4 },
	{8*8+0,8*8+1,8*8+2,8*8+3,0,1,2,3},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7},
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0,4 },
	{8*8+0,8*8+1,8*8+2,8*8+3,0,1,2,3,

	16*8+8*8+0,16*8+8*8+1,16*8+8*8+2,16*8+8*8+3,16*8+0,16*8+1,16*8+2,16*8+3},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7,
	16*8*2+8*0, 16*8*2+8*1, 16*8*2+8*2, 16*8*2+8*3, 16*8*2+8*4, 16*8*2+8*5, 16*8*2+8*6, 16*8*2+8*7,

	},
	16*8*4
};

static GFXDECODE_START( gfx_superwng )
	GFXDECODE_ENTRY( "gfx", 0x0000, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx", 0x0000, spritelayout, 0, 16 )
GFXDECODE_END

void superwng_state::machine_start()
{
	save_item(NAME(m_tile_bank));
	save_item(NAME(m_nmi_enable));
	m_mainbank->configure_entries(0, 2, memregion("maincpu")->base() + 0x4000, 0x4000);
}

void superwng_state::machine_reset()
{
	m_nmi_enable = 0;
}

void superwng_state::superwng(machine_config &config)
{
	static constexpr XTAL MASTER_CLOCK = 18.432_MHz_XTAL;

	// basic machine hardware
	Z80(config, m_maincpu, MASTER_CLOCK / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &superwng_state::main_map);

	Z80(config, m_audiocpu, MASTER_CLOCK / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &superwng_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(superwng_state::sound_nmi_assert), attotime::from_hz(4 * 60));

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, 0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(superwng_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(superwng_state::main_nmi_interrupt));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_superwng);

	PALETTE(config, m_palette, FUNC(superwng_state::palette), 0x40);

	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", MASTER_CLOCK / 12));
	ay1.port_a_read_callback().set("soundlatch", FUNC(generic_latch_8_device::read));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.50);

	AY8910(config, "ay2", MASTER_CLOCK / 12).add_route(ALL_OUTPUTS, "mono", 0.50);
}

ROM_START( superwng )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "2.5l", 0x0000, 0x2000, CRC(8d102f8d) SHA1(ff6d994273a2e493a68637822cd0b1a2f69fd054) )
	ROM_LOAD( "3.5m", 0x2000, 0x2000, CRC(3b08bd19) SHA1(2020e2835df86a6a279bbf9d013a489f0e32a4bd) )
	ROM_LOAD( "4.5p", 0x4000, 0x2000, CRC(6a49746d) SHA1(f5cd5eb77f60972a3897243f9ee3d61aac0878fc) )
	ROM_LOAD( "5.5r", 0x6000, 0x2000, CRC(ebd23487) SHA1(16e8faf989aa80dbf9934450ec4ba642a6f88c63) )
	ROM_LOAD( "6.8s", 0x8000, 0x4000, BAD_DUMP CRC(774433e0) SHA1(82b10d797581c14914bcce320f2aa5d3fb1fba33) ) // banked but probably bad, bits at 0xxx39 offsets appear to be missing / corrupt.

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "1.1a", 0x0000, 0x2000, CRC(a70aa39e) SHA1(b03de65d7bd020eb77495997128dce5ccbdbefac) )

	ROM_REGION( 0x4000, "gfx", 0 )
	ROM_LOAD( "7.8p", 0x0000, 0x4000, CRC(b472603c) SHA1(96f477a47a5be3db1292fea4f5c91ab155013f74) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "bprom.bin", 0x0000, 0x0040, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1985, superwng, 0, superwng, superwng, superwng_state, empty_init, ROT90, "Wing", "Super Wing", MACHINE_NOT_WORKING | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE ) // crashes after bonus stage, see notes, bad ROM?
