// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**********************************************************************************************

    Mirage Youjuu Mahjongden (c) 1994 Mitchell

    Similar to simpl156.cpp with shifted stuff around.

    TODO:
    - some unknown writes (irq acks / sprite DMA start presumably);

    Notes:
    - To enter into full Test Mode you need to keep pressed the Mahjong A key at start-up.

===============================================================================================

Mirage Youjuu Mahjongden
(c)1994 Mitchell

MT4001-2
DEC-22V0
all custom chips are surface scratched, but I believe they are DECO156 and mates.

Sound: M6295x2
OSC  : 28.0000MHz

MBL-00.7A    [2e258b7b]

MBL-01.11A   [895be69a]
MBL-02.12A   [474f6104]

MBL-03.10A   [4a599703]

MBL-04.12K   [b533123d]

MR_00-.2A    [3a53f33d]
MR_01-.3A    [a0b758aa]

***********************************************************************************************/

#include "emu.h"
#include "deco16ic.h"
#include "decocrpt.h"
#include "decospr.h"

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class miragemj_state : public driver_device
{
public:
	miragemj_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_deco_tilegen(*this, "tilegen"),
		m_eeprom(*this, "eeprom"),
		m_oki_sfx(*this, "oki_sfx"),
		m_oki_bgm(*this, "oki_bgm"),
		m_spriteram(*this, "spriteram") ,
		m_pf_rowscroll(*this, "pf%u_rowscroll", 1U),
		m_sprgen(*this, "spritegen"),
		m_io_key(*this, "KEY%u", 0U)
	{ }

	void mirage(machine_config &config);

	void init_mirage();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void key_matrix_w(uint16_t data);
	uint16_t key_matrix_r();
	void okim1_rombank_w(uint16_t data);
	void okim0_rombank_w(uint16_t data);
	DECOSPR_PRIORITY_CB_MEMBER(pri_callback);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECO16IC_BANK_CB_MEMBER(bank_callback);
	void main_map(address_map &map) ATTR_COLD;

	/* devices */
	required_device<m68000_device> m_maincpu;
	required_device<deco16ic_device> m_deco_tilegen;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<okim6295_device> m_oki_sfx;
	required_device<okim6295_device> m_oki_bgm;
	required_device<buffered_spriteram16_device> m_spriteram;

	/* memory pointers */
	required_shared_ptr_array<uint16_t, 2> m_pf_rowscroll;
	optional_device<decospr_device> m_sprgen;

	required_ioport_array<5> m_io_key;

	/* misc */
	uint8_t m_key_matrix_select = 0;

};

uint32_t miragemj_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t const flip = m_deco_tilegen->pf_control_r(0);

	flip_screen_set(BIT(flip, 7));
	m_sprgen->set_flip_screen(BIT(flip, 7));

	m_deco_tilegen->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);

	screen.priority().fill(0, cliprect);
	bitmap.fill(256, cliprect); /* not verified */

	m_deco_tilegen->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
	m_deco_tilegen->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0x400);
	return 0;
}


void miragemj_state::key_matrix_w(uint16_t data)
{
	m_key_matrix_select = data & 0x1f;
}

uint16_t miragemj_state::key_matrix_r()
{
	uint16_t result = 0xffff;
	for (int i = 0; m_io_key.size() > i; ++i)
	{
		if (BIT(m_key_matrix_select, i))
			result &= m_io_key[i]->read();
	}
	return result;
}

void miragemj_state::okim1_rombank_w(uint16_t data)
{
	m_oki_sfx->set_rom_bank(data & 0x3);
}

void miragemj_state::okim0_rombank_w(uint16_t data)
{
	m_eeprom->clk_write(BIT(data, 5) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write(BIT(data, 4));
	m_eeprom->cs_write(BIT(data, 6) ? ASSERT_LINE : CLEAR_LINE);

	/* bits 4-6 used on POST? */
	m_oki_bgm->set_rom_bank(data & 0x7);
}

void miragemj_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	/* tilemaps */
	map(0x100000, 0x101fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w)); // 0x100000 - 0x101fff tested
	map(0x102000, 0x103fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w)); // 0x102000 - 0x102fff tested
	/* linescroll */
	map(0x110000, 0x110bff).ram().share(m_pf_rowscroll[0]);
	map(0x112000, 0x112bff).ram().share(m_pf_rowscroll[1]);
	map(0x120000, 0x1207ff).ram().share("spriteram");
	map(0x130000, 0x1307ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x140000, 0x14000f).rw(m_oki_sfx, FUNC(okim6295_device::read), FUNC(okim6295_device::write)).umask16(0x00ff);
	map(0x150000, 0x15000f).rw(m_oki_bgm, FUNC(okim6295_device::read), FUNC(okim6295_device::write)).umask16(0x00ff);
	map(0x160000, 0x160001).nopw();
	map(0x168000, 0x16800f).w(m_deco_tilegen, FUNC(deco16ic_device::pf_control_w));
	map(0x16a000, 0x16a001).nopw();
	map(0x16c000, 0x16c001).w(FUNC(miragemj_state::okim1_rombank_w));
	map(0x16c002, 0x16c003).w(FUNC(miragemj_state::okim0_rombank_w));
	map(0x16c004, 0x16c005).w(FUNC(miragemj_state::key_matrix_w));
	map(0x16c006, 0x16c007).r(FUNC(miragemj_state::key_matrix_r));
	map(0x16e000, 0x16e001).nopw();
	map(0x16e002, 0x16e003).portr("SYSTEM_IN");
	map(0x170000, 0x173fff).ram();
}


static INPUT_PORTS_START( mirage )
	PORT_START("SYSTEM_IN")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0xfff7, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout tile_8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	8*16
};

static const gfx_layout tile_16x16_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ STEP8(8*2*16,1),STEP8(0,1) },
	{ STEP16(0,8*2) },
	32*16
};

static GFXDECODE_START( gfx_mirage )
	GFXDECODE_ENTRY("tiles", 0, tile_8x8_layout,   0x000, 32)  /* Tiles (8x8) */
	GFXDECODE_ENTRY("tiles", 0, tile_16x16_layout, 0x000, 32)  /* Tiles (16x16) */
GFXDECODE_END

static GFXDECODE_START( gfx_mirage_spr )
	GFXDECODE_ENTRY("sprites", 0, tile_16x16_layout, 0x200, 32)  /* Sprites (16x16) */
GFXDECODE_END


DECOSPR_PRIORITY_CB_MEMBER(miragemj_state::pri_callback)
{
	u32 mask = 0; // above everything
	if (extpri)
		mask |= GFX_PMASK_2; // under the uppermost playfield

	return mask;
}

DECO16IC_BANK_CB_MEMBER(miragemj_state::bank_callback)
{
	return ((bank >> 4) & 0x7) * 0x1000;
}

void miragemj_state::machine_start()
{
	save_item(NAME(m_key_matrix_select));
}

void miragemj_state::machine_reset()
{
	m_key_matrix_select = 0;
}

void miragemj_state::mirage(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 28000000/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &miragemj_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(miragemj_state::irq6_line_hold));

	EEPROM_93C46_16BIT(config, "eeprom");  // 93C45

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(529));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(miragemj_state::screen_update));
	screen.screen_vblank().set(m_spriteram, FUNC(buffered_spriteram16_device::vblank_copy_rising));

	GFXDECODE(config, "gfxdecode", "palette", gfx_mirage);
	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 1024);

	DECO16IC(config, m_deco_tilegen, 0);
	m_deco_tilegen->set_pf1_size(DECO_64x32);
	m_deco_tilegen->set_pf2_size(DECO_64x32);
	m_deco_tilegen->set_pf1_col_bank(0x00);
	m_deco_tilegen->set_pf2_col_bank(0x10);
	m_deco_tilegen->set_pf1_col_mask(0x0f);
	m_deco_tilegen->set_pf2_col_mask(0x0f);
	m_deco_tilegen->set_bank1_callback(FUNC(miragemj_state::bank_callback));
	m_deco_tilegen->set_bank2_callback(FUNC(miragemj_state::bank_callback));
	m_deco_tilegen->set_pf12_8x8_bank(0);
	m_deco_tilegen->set_pf12_16x16_bank(1);
	m_deco_tilegen->set_gfxdecode_tag("gfxdecode");

	DECO_SPRITE(config, m_sprgen, 0, "palette", gfx_mirage_spr);
	m_sprgen->set_pri_callback(FUNC(miragemj_state::pri_callback));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki_bgm, 2000000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.30); // clock frequency & pin 7 not verified

	OKIM6295(config, m_oki_sfx, 1000000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.70); // clock frequency & pin 7 not verified
}


ROM_START( mirage )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "mr_00-.2a", 0x00000, 0x40000, CRC(3a53f33d) SHA1(0f654021dcd64202b41e0ef5ef3cdf5dd274f8a5) )
	ROM_LOAD16_BYTE( "mr_01-.3a", 0x00001, 0x40000, CRC(a0b758aa) SHA1(7fb5faf6fb57cd72a3ac24b8af1f33e504ac8398) )

	ROM_REGION( 0x100000, "tiles", 0 ) /* Tiles - Encrypted */
	ROM_LOAD( "mbl-00.7a", 0x000000, 0x100000, CRC(2e258b7b) SHA1(2dbd7d16a1eda97ae3de149b67e80e511aa9d0ba) )

	ROM_REGION( 0x400000, "sprites", 0 ) /* Sprites */
	ROM_LOAD( "mbl-01.11a", 0x200000, 0x200000, CRC(895be69a) SHA1(541d8f37fb4cf99312b80a0eb0d729fbbeab5f4f) )
	ROM_LOAD( "mbl-02.12a", 0x000000, 0x200000, CRC(474f6104) SHA1(ff81b32b90192c3d5f27c436a9246aa6caaeeeee) )

	ROM_REGION( 0x200000, "oki_bgm_data", 0 )
	ROM_LOAD( "mbl-03.10a", 0x000000, 0x200000, CRC(4a599703) SHA1(b49e84faa2d6acca952740d30fc8d1a33ac47e79) )

	ROM_REGION( 0x200000, "oki_bgm", 0 )
	ROM_COPY( "oki_bgm_data", 0x000000, 0x000000, 0x080000 )
	ROM_COPY( "oki_bgm_data", 0x100000, 0x080000, 0x080000 ) // - banks 2,3 and 4,5 are swapped, PAL address shuffle
	ROM_COPY( "oki_bgm_data", 0x080000, 0x100000, 0x080000 ) // /
	ROM_COPY( "oki_bgm_data", 0x180000, 0x180000, 0x080000 )

	ROM_REGION( 0x100000, "oki_sfx", 0 )    /* M6295 samples */
	ROM_LOAD( "mbl-04.12k", 0x000000, 0x100000, CRC(b533123d) SHA1(2cb2f11331d00c2d282113932ed2836805f4fc6e) )
ROM_END

void miragemj_state::init_mirage()
{
	deco56_decrypt_gfx(machine(), "tiles");
}

} // anonymous namespace


GAME( 1994, mirage, 0,     mirage, mirage, miragemj_state, init_mirage, ROT0, "Mitchell", "Mirage Youjuu Mahjongden (Japan)", MACHINE_SUPPORTS_SAVE )
