// license:BSD-3-Clause
// copyright-holders:Joseba Epalza, Andreas Naive

/***************************************************************************

 Speed Ball / Music Ball

 this was available in a number of cabinet types including 'Super Pin-Ball'
 which mimicked a Pinball table in design, complete with 7-seg scoreboard.

driver by Joseba Epalza

- 4MHz XTAL, 20MHz XTAL
- Z80 main CPU (4MHz)
- Z80 sound CPU (4MHz)
- YM3812

Video frequency is ~56.4Hz
Interrupt frequency on audio CPU is not a periodical signal, but there are a lot of pulses of 1 MHz

 ======================================================================

  Colors : 2 bits for foreground characters =  4 colors * 16 palettes
           4 bits for background tiles      = 16 colors * 16 palettes
           4 bits for sprites               = 16 colors * 16 palettes

 Note:
 - To enter test mode, keep pressed COIN1 and COIN2 during boot,
   until the RAM / ROM tests are finished

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "speedbal.lh"


// configurable logging
#define LOG_UNKWRITE     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_UNKWRITE)

#include "logmacro.h"

#define LOGUNKWRITE(...)     LOGMASKED(LOG_UNKWRITE,     __VA_ARGS__)


namespace {

class speedbal_state : public driver_device
{
public:
	speedbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_spriteram(*this, "spriteram")
		, m_background_videoram(*this, "bg_videoram")
		, m_foreground_videoram(*this, "fg_videoram")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void speedbal(machine_config &config);

	void init_speedbal();
	void init_musicbal();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_background_videoram;
	required_shared_ptr<uint8_t> m_foreground_videoram;
	output_finder<73> m_digits;

	bool m_leds_start = 0;
	uint32_t m_leds_shiftreg = 0U;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	void coincounter_w(uint8_t data);
	void foreground_videoram_w(offs_t offset, uint8_t data);
	void background_videoram_w(offs_t offset, uint8_t data);
	void maincpu_50_w(uint8_t data);
	void leds_output_block(uint8_t data);
	void leds_start_block(uint8_t data);
	void leds_shift_bit(uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info_bg);
	TILE_GET_INFO_MEMBER(get_tile_info_fg);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_cpu_io_map(address_map &map) ATTR_COLD;
	void main_cpu_map(address_map &map) ATTR_COLD;
	void sound_cpu_io_map(address_map &map) ATTR_COLD;
	void sound_cpu_map(address_map &map) ATTR_COLD;
};


TILE_GET_INFO_MEMBER(speedbal_state::get_tile_info_bg)
{
	int const code = m_background_videoram[tile_index * 2] + ((m_background_videoram[tile_index * 2 + 1] & 0x30) << 4);
	int const color = m_background_videoram[tile_index * 2 + 1] & 0x0f;

	tileinfo.set(1, code, color, 0);
	tileinfo.group = (color == 8);
}

TILE_GET_INFO_MEMBER(speedbal_state::get_tile_info_fg)
{
	int const code = m_foreground_videoram[tile_index * 2] + ((m_foreground_videoram[tile_index * 2 + 1] & 0x30) << 4);
	int const color = m_foreground_videoram[tile_index * 2 + 1] & 0x0f;

	tileinfo.set(0, code, color, 0);
	tileinfo.group = (color == 9);
}

/*************************************
 *                                   *
 *      Start-Stop                   *
 *                                   *
 *************************************/

void speedbal_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(speedbal_state::get_tile_info_bg)), TILEMAP_SCAN_COLS_FLIP_X, 16, 16, 16, 16);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(speedbal_state::get_tile_info_fg)), TILEMAP_SCAN_COLS_FLIP_X,  8,  8, 32, 32);

	m_bg_tilemap->set_transmask(0, 0xffff, 0x0000); // split type 0 is totally transparent in front half
	m_bg_tilemap->set_transmask(1, 0x00f7, 0x0000); // split type 1 has pen 0-2, 4-7 transparent in front half

	m_fg_tilemap->set_transmask(0, 0xffff, 0x0001); // split type 0 is totally transparent in front half and has pen 0 transparent in back half
	m_fg_tilemap->set_transmask(1, 0x0001, 0x0001); // split type 1 has pen 0 transparent in front and back half
}



/*************************************
 *                                   *
 *  Foreground characters RAM        *
 *                                   *
 *************************************/

void speedbal_state::foreground_videoram_w(offs_t offset, uint8_t data)
{
	m_foreground_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset >> 1);
}

/*************************************
 *                                   *
 *  Background tiles RAM             *
 *                                   *
 *************************************/

void speedbal_state::background_videoram_w(offs_t offset, uint8_t data)
{
	m_background_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset >> 1);
}


/*************************************
 *                                   *
 *   Sprite drawing                  *
 *                                   *
 *************************************/

void speedbal_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Drawing sprites: 64 in total

	for (int offset = 0; offset < m_spriteram.bytes(); offset += 4)
	{
		if (!(m_spriteram[offset + 2] & 0x80))
			continue;

		int x = 243 - m_spriteram[offset + 3];
		int y = 239 - m_spriteram[offset + 0];

		int const code = (m_spriteram[offset + 1]) | ((m_spriteram[offset + 2] & 0x40) << 2);

		int const color = m_spriteram[offset + 2] & 0x0f;

		int flipx = 0, flipy = 0;

		if (flip_screen())
		{
			x = 246 - x;
			y = 238 - y;
			flipx = flipy = 1;
		}

		m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				x, y, 0);
	}
}

/*************************************
 *                                   *
 *   Refresh screen                  *
 *                                   *
 *************************************/

uint32_t speedbal_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	return 0;
}


void speedbal_state::machine_start()
{
	m_digits.resolve();
	save_item(NAME(m_leds_start));
	save_item(NAME(m_leds_shiftreg));
}

void speedbal_state::coincounter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x80);
	machine().bookkeeping().coin_counter_w(1, data & 0x40);
	flip_screen_set(data & 8); // also changes data & 0x10 at the same time too (flipx and flipy?)
	// unknown: (data & 0x10) and (data & 4)
}

void speedbal_state::main_cpu_map(address_map &map)
{
	map(0x0000, 0xdbff).rom();
	map(0xdc00, 0xdfff).ram().share("share1"); // shared with SOUND
	map(0xe000, 0xe1ff).ram().w(FUNC(speedbal_state::background_videoram_w)).share(m_background_videoram);
	map(0xe800, 0xefff).ram().w(FUNC(speedbal_state::foreground_videoram_w)).share(m_foreground_videoram);
	map(0xf000, 0xf5ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xf600, 0xfeff).ram();
	map(0xff00, 0xffff).ram().share(m_spriteram);
}

void speedbal_state::maincpu_50_w(uint8_t data)
{
	LOGUNKWRITE("%s: maincpu_50_w %02x\n", machine().describe_context(), data);
}

void speedbal_state::main_cpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW2");
	map(0x10, 0x10).portr("DSW1");
	map(0x20, 0x20).portr("P1");
	map(0x30, 0x30).portr("P2");
	map(0x40, 0x40).w(FUNC(speedbal_state::coincounter_w));
	map(0x50, 0x50).w(FUNC(speedbal_state::maincpu_50_w));
}

void speedbal_state::sound_cpu_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xd800, 0xdbff).ram();
	map(0xdc00, 0xdfff).ram().share("share1"); // shared with MAIN CPU
}



void speedbal_state::leds_output_block(uint8_t data)
{
	if (!m_leds_start)
		return;

	m_leds_start = false;

	// Each hypothetical led block has 3 7seg leds.
	// The shift register is 28 bits, led block number is in the upper bits
	// and the other 3 bytes in it go to each 7seg led of the current block.
	int const block = m_leds_shiftreg >> 24 & 7;
	m_digits[10 * block] = ~m_leds_shiftreg & 0xff;
	m_digits[10 * block + 1] = ~m_leds_shiftreg >> 8 & 0xff;
	m_digits[10 * block + 2] = ~m_leds_shiftreg >> 16 & 0xff;
}

void speedbal_state::leds_start_block(uint8_t data)
{
	m_leds_shiftreg = 0;
	m_leds_start = true;
}

void speedbal_state::leds_shift_bit(uint8_t data)
{
	m_leds_shiftreg <<= 1;
	m_leds_shiftreg |= (data & 1);
}



void speedbal_state::sound_cpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0x40, 0x40).w(FUNC(speedbal_state::leds_output_block));
	map(0x80, 0x80).w(FUNC(speedbal_state::leds_start_block));
	map(0x82, 0x82).nopw(); // ?
	map(0xc1, 0xc1).w(FUNC(speedbal_state::leds_shift_bit));
}


static INPUT_PORTS_START( speedbal )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x06, "70000 200000 1M" )
	PORT_DIPSETTING(    0x07, "70000 200000" )
	PORT_DIPSETTING(    0x03, "100000 300000 1M" )
	PORT_DIPSETTING(    0x04, "100000 300000" )
	PORT_DIPSETTING(    0x01, "200000 1M" )
	PORT_DIPSETTING(    0x05, "200000" )
	PORT_DIPSETTING(    0x02, "200000 (duplicate)" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Difficulty 1" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Difficulty 2" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW , IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW , IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW , IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW , IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW , IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW , IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW , IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW , IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW , IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW , IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW , IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW , IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW , IPT_TILT    )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( musicbal )
	PORT_INCLUDE(speedbal)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x03, "1M 2M 2.5M" )
	PORT_DIPSETTING(    0x06, "1.2M 1.8M 2.5M" )
	PORT_DIPSETTING(    0x07, "1.2M 1.8M" )
	PORT_DIPSETTING(    0x04, "1.5M 2M" )
	PORT_DIPSETTING(    0x05, "1.5M" )
	PORT_DIPSETTING(    0x01, "1.8M 2.5M" )
	PORT_DIPSETTING(    0x02, "1.8M" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	RGN_FRAC(1,2),   // 1024 characters
	4,      // actually 2 bits per pixel - two of the planes are empty
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 8+3, 8+2, 8+1, 8+0, 3, 2, 1, 0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },   // characters are rotated 90 degrees
	16*8       // every char takes 16 bytes
};

static const gfx_layout tilelayout =
{
	16,16,  // 16*16 tiles
	RGN_FRAC(1,1),   // 1024 tiles
	4,      // 4 bits per pixel
	{ 0, 2, 4, 6 }, // the bitplanes are packed in one nibble
	{ 0*8+0, 0*8+1, 7*8+0, 7*8+1, 6*8+0, 6*8+1, 5*8+0, 5*8+1,
			4*8+0, 4*8+1, 3*8+0, 3*8+1, 2*8+0, 2*8+1, 1*8+0, 1*8+1 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8  // every sprite takes 128 consecutive bytes
};

static const gfx_layout spritelayout =
{
	16,16,  // 16*16 sprites
	RGN_FRAC(1,1),    // 512 sprites
	4,      // 4 bits per pixel
	{ 0, 2, 4, 6 }, // the bitplanes are packed in one nibble
	{ 7*8+1, 7*8+0, 6*8+1, 6*8+0, 5*8+1, 5*8+0, 4*8+1, 4*8+0,
			3*8+1, 3*8+0, 2*8+1, 2*8+0, 1*8+1, 1*8+0, 0*8+1, 0*8+0 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8  // every sprite takes 128 consecutive bytes
};

static GFXDECODE_START( gfx_speedbal )
	GFXDECODE_ENTRY( "chars",   0, charlayout,   256, 16 )
	GFXDECODE_ENTRY( "bgtiles", 0, tilelayout,   512, 16 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,   0, 16 )
GFXDECODE_END



void speedbal_state::speedbal(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &speedbal_state::main_cpu_map);
	m_maincpu->set_addrmap(AS_IO, &speedbal_state::main_cpu_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(speedbal_state::irq0_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(4'000'000)));
	audiocpu.set_addrmap(AS_PROGRAM, &speedbal_state::sound_cpu_map);
	audiocpu.set_addrmap(AS_IO, &speedbal_state::sound_cpu_io_map);
	audiocpu.set_periodic_int(FUNC(speedbal_state::irq0_line_hold), attotime::from_hz(1000 / 2)); // approximate?

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(56.4); // measured
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(speedbal_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_speedbal);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 768).set_endianness(ENDIANNESS_BIG);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM3812(config, "ymsnd", XTAL(4'000'000)).add_route(ALL_OUTPUTS, "mono", 1.0); // 4 MHz(?)
}


void speedbal_state::init_speedbal()
{
	// sprite tiles are in an odd order, rearrange to simplify video drawing function
	uint8_t *rom = memregion("sprites")->base();
	uint8_t temp[0x200 * 128];

	for (int i = 0; i < 0x200; i++)
	{
		int j = bitswap<16>(i, 15, 14, 13, 12, 11, 10, 9, 8, 0, 1, 2, 3, 4, 5, 6, 7);
		memcpy(temp + i * 128, rom + j * 128, 128);
	}

	memcpy(rom, temp, 0x200 * 128);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( speedbal ) // seems to have a more complete hidden test mode, with a 'hard test' that's not enabled in the alternate Speed Ball ROM set
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.u14",  0x0000,  0x8000, CRC(94c6f107) SHA1(cd7ada17f0f59623cf615df68c5f8f4077377820) )
	ROM_LOAD( "3.u15",  0x8000,  0x8000, CRC(a036687f) SHA1(fc2cd683cd6a9a75ab6b188f7b4592b355a569e0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "2.u100",  0x0000,  0x8000, CRC(e6a6d9b7) SHA1(35d228d13d4305f606fdd84adad1d6e435f4b7ce) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "10.u50", 0x00000, 0x08000, CRC(36dea4bf) SHA1(60095f482af4595a39be5ae6def8cd30298c1ef8) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "9.u45",  0x00000, 0x08000, CRC(b567e85e) SHA1(7036792ea70ad48384f348399ed9b136272fedb6) )
	ROM_LOAD( "5.u46",  0x08000, 0x08000, CRC(b0eae4ba) SHA1(baee3fcb1399c56efaa5f97912de324d7b38f286) )
	ROM_LOAD( "8.u47",  0x10000, 0x08000, CRC(d2bfbdb6) SHA1(b552b055450f438729c83337f561d05b6518ae75) )
	ROM_LOAD( "4.u48",  0x18000, 0x08000, CRC(1d23a130) SHA1(aabf7c46f9299ffb8b8ca92839622d000a470a0b) )

	ROM_REGION( 0x10000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "7.u67",  0x00000, 0x08000, CRC(9f1b33d1) SHA1(1f8be8f8e6a2ee99a7dafeead142ccc629fa792d) )
	ROM_LOAD( "6.u68",  0x08000, 0x08000, CRC(0e2506eb) SHA1(56f779266b977819063c475b84ca246fc6d8d6a7) )
ROM_END

ROM_START( speedbala )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sb1.bin",  0x0000,  0x8000, CRC(1c242e34) SHA1(8b2e8983e0834c99761ce2b5ea765dba56e77964) )
	ROM_LOAD( "sb3.bin",  0x8000,  0x8000, CRC(7682326a) SHA1(15a72bf088a9adfaa50c11202b4970e07c309a21) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sb2.bin",  0x0000,  0x8000, CRC(e6a6d9b7) SHA1(35d228d13d4305f606fdd84adad1d6e435f4b7ce) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "sb10.bin", 0x00000, 0x08000, CRC(36dea4bf) SHA1(60095f482af4595a39be5ae6def8cd30298c1ef8) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "sb9.bin",  0x00000, 0x08000, CRC(b567e85e) SHA1(7036792ea70ad48384f348399ed9b136272fedb6) )
	ROM_LOAD( "sb5.bin",  0x08000, 0x08000, CRC(b0eae4ba) SHA1(baee3fcb1399c56efaa5f97912de324d7b38f286) )
	ROM_LOAD( "sb8.bin",  0x10000, 0x08000, CRC(d2bfbdb6) SHA1(b552b055450f438729c83337f561d05b6518ae75) )
	ROM_LOAD( "sb4.bin",  0x18000, 0x08000, CRC(1d23a130) SHA1(aabf7c46f9299ffb8b8ca92839622d000a470a0b) )

	ROM_REGION( 0x10000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "sb7.bin",  0x00000, 0x08000, CRC(9f1b33d1) SHA1(1f8be8f8e6a2ee99a7dafeead142ccc629fa792d) )
	ROM_LOAD( "sb6.bin",  0x08000, 0x08000, CRC(0e2506eb) SHA1(56f779266b977819063c475b84ca246fc6d8d6a7) )
ROM_END

ROM_START( musicbal )
	ROM_REGION( 0x10000, "maincpu", 0 ) // encrypted
	ROM_LOAD( "01.bin",  0x0000,  0x8000, CRC(412298a2) SHA1(3c3247b466880cd78dd7f7f73911f475352c15df) )
	ROM_LOAD( "03.bin",  0x8000,  0x8000, CRC(fdf14446) SHA1(9e52810ebc2b18d83f349fb78884b3c380d93903) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "02.bin",  0x0000,  0x8000, CRC(b7d3840d) SHA1(825289c3ca51284a47cfc4937a18d098183c396a) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "10.bin",  0x00000, 0x08000, CRC(5afd3c42) SHA1(5b9a44ef03e5519c9601bb636eb26768cf800278) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "09.bin",  0x00000, 0x08000, CRC(dcde4233) SHA1(99f204ddc97ee45330ea3cb0bc2971cd95f8e1ac) )
	ROM_LOAD( "05.bin",  0x08000, 0x08000, CRC(e1eec437) SHA1(ce77f3bb01db80ec69da9639d193d8656f9c6692) )
	ROM_LOAD( "08.bin",  0x10000, 0x08000, CRC(7e7af52b) SHA1(3bb1c5abfb1fe53f01520e93124708df6750d8b5) )
	ROM_LOAD( "04.bin",  0x18000, 0x08000, CRC(bf931a33) SHA1(b2ab5c6103af0e0508f08fd58b425e5acfe9ef8a) )

	ROM_REGION( 0x10000, "sprites", ROMREGION_INVERT ) // still contain Speed Ball logos!
	ROM_LOAD( "07.bin",  0x00000, 0x08000, CRC(310e1e23) SHA1(290f3e1c7b907165fe60a4ebe7a8b04b2451b3b1) )
	ROM_LOAD( "06.bin",  0x08000, 0x08000, CRC(2e7772f8) SHA1(caded1a72356501282e627e23718c30cb8f09370) )
ROM_END


void speedbal_state::init_musicbal()
{
	uint8_t *rom = memregion("maincpu")->base();

	const uint8_t xorTable[8] = {0x05, 0x06, 0x84, 0x84, 0x00, 0x87, 0x84, 0x84};     // XORs affecting bits #0, #1, #2 & #7
	const int swapTable[4][4] = {                                                   // 4 possible swaps affecting bits #0, #1, #2 & #7
		{1,0,7,2},
		{2,7,0,1},
		{7,2,1,0},
		{0,2,1,7}
	};

	for (int i = 0; i < 0x8000; i++)
	{
		int addIdx = BIT(i, 3) ^ (BIT(i, 5) << 1) ^ (BIT(i, 9) << 2);  // 3 bits of address...
		int xorMask = xorTable[addIdx];                                // ... control the XOR...
		int bswIdx = xorMask & 3;                                      // ... and the bitswap

		// only bits #0, #1, #2 & #7 are affected
		rom[i] = bitswap<8>(rom[i], swapTable[bswIdx][3], 6, 5, 4, 3, swapTable[bswIdx][2], swapTable[bswIdx][1], swapTable[bswIdx][0]) ^ xorTable[addIdx];
	}

	init_speedbal();
}

} // anonymous namespace


GAMEL( 1987, speedbal,         0, speedbal, speedbal, speedbal_state, init_speedbal, ROT270, "Tecfri / Desystem S.A.", "Speed Ball (set 1)", MACHINE_SUPPORTS_SAVE, layout_speedbal )
GAMEL( 1987, speedbala, speedbal, speedbal, speedbal, speedbal_state, init_speedbal, ROT270, "Tecfri / Desystem S.A.", "Speed Ball (set 2)", MACHINE_SUPPORTS_SAVE, layout_speedbal )
GAMEL( 1988, musicbal,         0, speedbal, musicbal, speedbal_state, init_musicbal, ROT270, "Tecfri / Desystem S.A.", "Music Ball",         MACHINE_SUPPORTS_SAVE, layout_speedbal )
