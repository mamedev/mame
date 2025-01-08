// license:BSD-3-Clause
// copyright-holders: Zsolt Vasvari

/******************************************************************************

Super Locomotive

driver by Zsolt Vasvari

TODO:
- Bit 5 in control_w is pulsed when loco turns "super". This is supposed
  to make red parts of sprites blink to purple, it's not clear how this is
  implemented in hardware, there's a hack to support it.

Sega PCB 834-5137
 Sega 315-5015 (Sega custom encrypted Z80)
 Sega 315-5011
 Sega 315-5012
 Z80
 M5L8255AP
 8 switch Dipswitch x 2

******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/segacrpt_device.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_CTRLBIT4     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_CTRLBIT4)

#include "logmacro.h"

#define LOGCTRLBIT4(...)     LOGMASKED(LOG_CTRLBIT4,     __VA_ARGS__)


namespace {

class suprloco_state : public driver_device
{
public:
	suprloco_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_scrollram(*this, "scrollram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_sprites_rom(*this, "sprites")
	{ }

	void suprloco(machine_config &config);

	void init_suprloco();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_decrypted_opcodes;
	required_region_ptr<uint8_t> m_sprites_rom;

	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_control = 0;

	enum
	{
		SPR_Y_TOP = 0,
		SPR_Y_BOTTOM,
		SPR_X,
		SPR_COL,
		SPR_SKIP_LO,
		SPR_SKIP_HI,
		SPR_GFXOFS_LO,
		SPR_GFXOFS_HI
	};

	void videoram_w(offs_t offset, uint8_t data);
	void scrollram_w(offs_t offset, uint8_t data);
	void control_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void draw_pixel(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, int color, int flip);
	void draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, int spr_number);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

  I'm not sure about the resistor values, I'm using the Galaxian ones.

***************************************************************************/
void suprloco_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < 512; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// blue component
		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));

		// hack: generate a second bank of sprite palette with red changed to purple
		if (i >= 256)
			palette.set_pen_color(i + 256, rgb_t(r, g, ((i & 0x0f) == 0x09) ? 0xff : b));
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(suprloco_state::get_tile_info)
{
	uint8_t const attr = m_videoram[2 * tile_index + 1];
	tileinfo.set(0,
			m_videoram[2 * tile_index] | ((attr & 0x03) << 8),
			(attr & 0x1c) >> 2,
			0);
	tileinfo.category = (attr & 0x20) >> 5;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void suprloco_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(suprloco_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_scroll_rows(32);

	save_item(NAME(m_control));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void suprloco_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void suprloco_state::scrollram_w(offs_t offset, uint8_t data)
{
	int const adj = flip_screen() ? -8 : 8;

	m_scrollram[offset] = data;
	m_bg_tilemap->set_scrollx(offset, data - adj);
}

void suprloco_state::control_w(uint8_t data)
{
	/* There is probably a palette select in here

	   Bit 0   - coin counter A
	   Bit 1   - coin counter B (only used if coinage differs from A)
	   Bit 2-3 - probably unused
	   Bit 4   - ???
	   Bit 5   - pulsated when loco turns "super"
	   Bit 6   - probably unused
	   Bit 7   - flip screen */

	if ((m_control & 0x10) != (data & 0x10))
		LOGCTRLBIT4("Bit 4 = %d\n", (data >> 4) & 1);

	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	flip_screen_set(data & 0x80);

	m_control = data;
}



inline void suprloco_state::draw_pixel(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, int color, int flip)
{
	if (flip)
	{
		x = bitmap.width() - x - 1;
		y = bitmap.height() - y - 1;
	}

	if (cliprect.contains(x, y))
		bitmap.pix(y, x) = color;
}


void suprloco_state::draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, int spr_number)
{
	int const flip = flip_screen();
	int adjy, dy;

	const uint8_t *spr_reg = m_spriteram + 0x10 * spr_number;

	int src = spr_reg[SPR_GFXOFS_LO] + (spr_reg[SPR_GFXOFS_HI] << 8);
	short const skip = spr_reg[SPR_SKIP_LO] + (spr_reg[SPR_SKIP_HI] << 8); // bytes to skip before drawing each row (can be negative)

	int const height = spr_reg[SPR_Y_BOTTOM] - spr_reg[SPR_Y_TOP];
	pen_t const pen_base = 0x100 + 0x10 * (spr_reg[SPR_COL] & 0x03) + ((m_control & 0x20) ? 0x100 : 0);
	int const sx = spr_reg[SPR_X];
	int const sy = spr_reg[SPR_Y_TOP] + 1;

	if (!flip)
	{
		adjy = sy;
		dy = 1;
	}
	else
	{
		adjy = sy + height - 1;  // some of the sprites are still off by a pixel
		dy = -1;
	}

	for (int row = 0; row < height; row++, adjy += dy)
	{
		int color1, color2;
		uint8_t data;

		src += skip;

		int col = 0;

		// get pointer to packed sprite data
		const uint8_t *gfx = &(m_sprites_rom[src & 0x7fff]);
		int const flipx = src & 0x8000;   // flip x

		while (1)
		{
			if (flipx)  // flip x
			{
				data = *gfx--;
				color1 = data & 0x0f;
				color2 = data >> 4;
			}
			else
			{
				data = *gfx++;
				color1 = data >> 4;
				color2 = data & 0x0f;
			}

			if (color1 == 15) break;
			if (color1)
				draw_pixel(bitmap, cliprect,sx + col, adjy, pen_base + color1, flip);

			if (color2 == 15) break;
			if (color2)
				draw_pixel(bitmap, cliprect,sx + col + 1, adjy, pen_base + color2, flip);

			col += 2;
		}
	}
}

void suprloco_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int spr_number = 0; spr_number < (m_spriteram.bytes() >> 4); spr_number++)
	{
		const uint8_t *spr_reg = m_spriteram + 0x10 * spr_number;
		if (spr_reg[SPR_X] != 0xff)
			draw_sprite(bitmap, cliprect, spr_number);
	}
}

uint32_t suprloco_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	return 0;
}


void suprloco_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc1ff).ram().share(m_spriteram);
	map(0xc200, 0xc7ff).nopw();
	map(0xc800, 0xc800).portr("SYSTEM");
	map(0xd000, 0xd000).portr("P1");
	map(0xd800, 0xd800).portr("P2");
	map(0xe000, 0xe000).portr("DSW1");
	map(0xe001, 0xe001).portr("DSW2");
	map(0xe800, 0xe803).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf000, 0xf6ff).ram().w(FUNC(suprloco_state::videoram_w)).share(m_videoram);
	map(0xf700, 0xf7df).ram(); // unused
	map(0xf7e0, 0xf7ff).ram().w(FUNC(suprloco_state::scrollram_w)).share(m_scrollram);
	map(0xf800, 0xffff).ram();
}

void suprloco_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().share(m_decrypted_opcodes);
	map(0x8000, 0xbfff).rom().region("maincpu", 0x8000);
}

void suprloco_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa003).w("sn1", FUNC(sn76496_device::write));
	map(0xc000, 0xc003).w("sn2", FUNC(sn76496_device::write));
	map(0xe000, 0xe000).r("ppi", FUNC(i8255_device::acka_r));
}



static INPUT_PORTS_START( suprloco )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0xc0, "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x01, "30000" )
	PORT_DIPSETTING(    0x02, "40000" )
	PORT_DIPSETTING(    0x03, "50000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )       PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, "Infinite Lives (Cheat)") PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Initial Entry" )     PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    // 8 by 8
	1024,   // 1024 characters
	4,      // 4 bits per pixel
	{ 0, 1024*8*8, 2*1024*8*8, 3*1024*8*8 },            // plane
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( gfx_suprloco )
	// sprites use colors 256-511 + 512-767
	GFXDECODE_ENTRY( "chars", 0x6000, charlayout, 0, 16 )
GFXDECODE_END


void suprloco_state::suprloco(machine_config &config)
{
	// basic machine hardware
	sega_315_5015_device &maincpu(SEGA_315_5015(config, m_maincpu, 4'000'000));   // 4 MHz (?)
	maincpu.set_addrmap(AS_PROGRAM, &suprloco_state::main_map);
	maincpu.set_addrmap(AS_OPCODES, &suprloco_state::decrypted_opcodes_map);
	maincpu.set_vblank_int("screen", FUNC(suprloco_state::irq0_line_hold));
	maincpu.set_decrypted_tag(":decrypted_opcodes");

	Z80(config, m_audiocpu, 4'000'000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &suprloco_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(suprloco_state::irq0_line_hold), attotime::from_hz(4 * 60));          // NMIs are caused by the main CPU

	i8255_device &ppi(I8255A(config, "ppi"));
	ppi.out_pb_callback().set(FUNC(suprloco_state::control_w));
	ppi.tri_pb_callback().set_constant(0);
	ppi.out_pc_callback().set_output("lamp0").bit(0).invert(); // set by 8255 bit mode when no credits inserted
	ppi.out_pc_callback().append_inputline(m_audiocpu, INPUT_LINE_NMI).bit(7).invert();

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(5000));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(1*8, 31*8-1, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(suprloco_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_suprloco);
	PALETTE(config, "palette", FUNC(suprloco_state::palette), 512 + 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SN76496(config, "sn1", 4'000'000).add_route(ALL_OUTPUTS, "mono", 1.0);

	SN76496(config, "sn2", 2'000'000).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( suprloco )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "epr-5226a.37",    0x0000, 0x4000, CRC(33b02368) SHA1(c6e3116ad4b52bcc3174de5770f7a7ce024790d5) ) // encrypted
	ROM_LOAD( "epr-5227a.15",    0x4000, 0x4000, CRC(a5e67f50) SHA1(1dd52e4cf00ce414fe1db8259c9976cdc23513b4) ) // encrypted
	ROM_LOAD( "epr-5228.28",     0x8000, 0x4000, CRC(a597828a) SHA1(61004d112591fd2d752c39df71c1304d9308daae) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-5222.64",     0x0000, 0x2000, CRC(0aa57207) SHA1(b29b533505cb5b47c90534f2f610baeb7265d030) )

	ROM_REGION( 0xe000, "chars", 0 )
	ROM_LOAD( "epr-5225.63",     0x0000, 0x2000, CRC(e571fe81) SHA1(ac2b5914a445b89b7456b2c4290e4630b525f05d) )
	ROM_LOAD( "epr-5224.62",     0x2000, 0x2000, CRC(6130f93c) SHA1(ae0657f46c10e75eec994e75359a89b5d61baf68) )
	ROM_LOAD( "epr-5223.61",     0x4000, 0x2000, CRC(3b03004e) SHA1(805b51cb14d3ace97f2e0f306db28921b2f5e322) )
							// 0x6000 - 0xe000 will be created by init_suprloco

	ROM_REGION( 0x8000, "sprites", 0 ) // used at runtime
	ROM_LOAD( "epr-5229.55",     0x0000, 0x4000, CRC(ee2d3ed3) SHA1(593f3cd5c4e7f20b5e31e6bac8864774442e4b75) )
	ROM_LOAD( "epr-5230.56",     0x4000, 0x2000, CRC(f04a4b50) SHA1(80363f0c508fb2a755bf684f9a6862c1e7285495) )
							// 0x6000 empty

	ROM_REGION( 0x0620, "proms", 0 )
	ROM_LOAD( "pr-5220.100",     0x0100, 0x0080, CRC(7b0c8ce5) SHA1(4e1ea5ce38198a3965dfeb609ba0c7e8211531c3) ) // color PROM
	ROM_CONTINUE(                0x0000, 0x0080 )
	ROM_CONTINUE(                0x0180, 0x0080 )
	ROM_CONTINUE(                0x0080, 0x0080 )
	ROM_LOAD( "pr-5219.89",      0x0200, 0x0400, CRC(1d4b02cb) SHA1(00d822f1bc4f57f2f5d5a0615241f8136246a842) ) // 3bpp to 4bpp table
	ROM_LOAD( "pr-5221.7",       0x0600, 0x0020, CRC(89ba674f) SHA1(17c87840c8011968675a5a6f55966467df02364b) ) // unknown
ROM_END

ROM_START( suprlocoo )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "epr-5226.37",     0x0000, 0x4000, CRC(57f514dd) SHA1(707800b90a22547a56b01d1e11775e9ee5555d23) ) // encrypted
	ROM_LOAD( "epr-5227.15",     0x4000, 0x4000, CRC(5a1d2fb0) SHA1(fdb9416e5530718245fd597073a63feddb233c3c) ) // encrypted
	ROM_LOAD( "epr-5228.28",     0x8000, 0x4000, CRC(a597828a) SHA1(61004d112591fd2d752c39df71c1304d9308daae) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-5222.64",     0x0000, 0x2000, CRC(0aa57207) SHA1(b29b533505cb5b47c90534f2f610baeb7265d030) )

	ROM_REGION( 0xe000, "chars", 0 )
	ROM_LOAD( "epr-5225.63",     0x0000, 0x2000, CRC(e571fe81) SHA1(ac2b5914a445b89b7456b2c4290e4630b525f05d) )
	ROM_LOAD( "epr-5224.62",     0x2000, 0x2000, CRC(6130f93c) SHA1(ae0657f46c10e75eec994e75359a89b5d61baf68) )
	ROM_LOAD( "epr-5223.61",     0x4000, 0x2000, CRC(3b03004e) SHA1(805b51cb14d3ace97f2e0f306db28921b2f5e322) )
							// 0x6000 - 0xe000 will be created by init_suprloco

	ROM_REGION( 0x8000, "sprites", 0 ) // used at runtime
	ROM_LOAD( "epr-5229.55",     0x0000, 0x4000, CRC(ee2d3ed3) SHA1(593f3cd5c4e7f20b5e31e6bac8864774442e4b75) )
	ROM_LOAD( "epr-5230.56",     0x4000, 0x2000, CRC(f04a4b50) SHA1(80363f0c508fb2a755bf684f9a6862c1e7285495) )
							// 0x6000 empty

	ROM_REGION( 0x0620, "proms", 0 )
	ROM_LOAD( "pr-5220.100",     0x0100, 0x0080, CRC(7b0c8ce5) SHA1(4e1ea5ce38198a3965dfeb609ba0c7e8211531c3) ) // color PROM
	ROM_CONTINUE(                0x0000, 0x0080 )
	ROM_CONTINUE(                0x0180, 0x0080 )
	ROM_CONTINUE(                0x0080, 0x0080 )
	ROM_LOAD( "pr-5219.89",      0x0200, 0x0400, CRC(1d4b02cb) SHA1(00d822f1bc4f57f2f5d5a0615241f8136246a842) ) // 3bpp to 4bpp table
	ROM_LOAD( "pr-5221.7",       0x0600, 0x0020, CRC(89ba674f) SHA1(17c87840c8011968675a5a6f55966467df02364b) ) // unknown
ROM_END

void suprloco_state::init_suprloco()
{
	// convert graphics to 4bpp from 3bpp
	uint8_t *source = memregion("chars")->base();
	uint8_t *dest   = source + 0x6000;
	uint8_t *lookup = memregion("proms")->base() + 0x0200;

	for (int i = 0; i < 0x80; i++, lookup += 8)
	{
		for (int j = 0; j < 0x40; j++, source++, dest++)
		{
			dest[0] = dest[0x2000] = dest[0x4000] = dest[0x6000] = 0;

			for (int k = 0; k < 8; k++)
			{
				const int color_source = (((source[0x0000] >> k) & 0x01) << 2) |
										 (((source[0x2000] >> k) & 0x01) << 1) |
										 (((source[0x4000] >> k) & 0x01) << 0);

				const int color_dest = lookup[color_source];

				dest[0x0000] |= (((color_dest >> 3) & 0x01) << k);
				dest[0x2000] |= (((color_dest >> 2) & 0x01) << k);
				dest[0x4000] |= (((color_dest >> 1) & 0x01) << k);
				dest[0x6000] |= (((color_dest >> 0) & 0x01) << k);
			}
		}
	}

}

} // anonymous namespace


GAME( 1982, suprloco,         0, suprloco, suprloco, suprloco_state, init_suprloco, ROT0, "Sega", "Super Locomotive (Rev.A)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, suprlocoo, suprloco, suprloco, suprloco, suprloco_state, init_suprloco, ROT0, "Sega", "Super Locomotive",         MACHINE_SUPPORTS_SAVE )
