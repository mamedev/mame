// license:BSD-3-Clause
// copyright-holders:

/*
    Blood Bros. (Modular System)

    As with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.

    For this game the Modular System cage contains 8 main boards and 1 sub board.

    MOD-6/1 - MC68000P10, 6 ROMs, RAMs, 22.1184 MHz XTAL.
    MOD 21/1(?) - 24 MHz XTAL.
    MOD 1/2 - Sound board (Z80, 2xYM2203C). 2 8-dips banks + small sub board with OKI M5205.
    MOD 51/1 - Sprite board, has logic + 4 empty ROM sockets. Sprite ROMs are actually on the below board.
    MODULAR SYSTEM 2 - red sprite ROM board, 16 sprite ROMs populated (maximum 24 ROMs)
    MOD 4/3 - Tilemap board, has logic + 4 27256 EPROMs, long thin sub-board (CAR-0484/1 SOLD) with no chips, just routing along one edge.
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (CAR-0484/1 SOLD) with no chips, just routing along one edge.
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (CAR-0484/1 SOLD) with no chips, just routing along one edge.

    PCBs pictures and dip listing are available at: http://www.recreativas.org/modular-system-blood-bros-4316-gaelco-sa

TODO:
- not sure if MSM5205 is playing all sounds it should
- sprite / tilemap priorities
*/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class bloodbro_ms_state : public driver_device
{
public:
	bloodbro_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_soundlatch(*this, "soundlatch%u", 0U),
		m_ym(*this, "ym%u", 0U),
		m_msm(*this, "msm"),
		m_spriteram(*this, "spriteram"),
		m_tileram(*this, "tileram%u", 0U),
		m_soundbank(*this, "sound_bank")
	{ }

	void bloodbrom(machine_config &config) ATTR_COLD;

	void init_bloodbrom() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;
	required_device_array<ym2203_device, 2> m_ym;
	required_device<msm5205_device> m_msm;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr_array<uint16_t, 3> m_tileram; // 0 bg, 1 fg, 2 tx
	required_memory_bank m_soundbank;

	bool m_audio_select = false;
	uint8_t m_adpcm_data = 0;

	tilemap_t *m_tilemap[3] {};
	uint16_t m_scrollreg[2] {};

	void adpcm_w(uint8_t data);
	void ym_w(offs_t offset, uint8_t data);
	void adpcm_int(int state);
	void soundlatch_w(uint8_t data);

	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_tile_info);
	template <uint8_t Which> void tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_program_map(address_map &map) ATTR_COLD;
	void sound_program_map(address_map &map) ATTR_COLD;

	void descramble_16x16tiles(uint8_t *src, int len) ATTR_COLD;
};


void bloodbro_ms_state::machine_start()
{
	m_soundbank->configure_entries(0, 2, memregion("audiocpu")->base() + 0x8000, 0x4000);
	m_soundbank->set_entry(0);

	save_item(NAME(m_audio_select));
	save_item(NAME(m_adpcm_data));
	save_item(NAME(m_scrollreg));
}


void bloodbro_ms_state::adpcm_w(uint8_t data)
{
	m_audio_select = BIT(data, 7);
	m_soundbank->set_entry(m_audio_select);
	m_msm->reset_w(BIT(data, 4));

	m_adpcm_data = data & 0x0f;
}

void bloodbro_ms_state::ym_w(offs_t offset, uint8_t data)
{
	if (!m_audio_select)
	{
		if (!BIT(offset, 1))
			m_ym[0]->write(offset & 1, data);
		else
			m_ym[1]->write(offset & 1, data);
	}
	else
	{
		// ??? (written during ADPCM IRQ)
	}
}

void bloodbro_ms_state::soundlatch_w(uint8_t data)
{
	m_soundlatch[0]->clear_w(data);
	m_soundlatch[1]->clear_w(data);
}

void bloodbro_ms_state::adpcm_int(int state)
{
	m_msm->data_w(m_adpcm_data);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

void bloodbro_ms_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bloodbro_ms_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 16);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bloodbro_ms_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 16);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bloodbro_ms_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_tilemap[1]->set_transparent_pen(0x0f);
	m_tilemap[2]->set_transparent_pen(0x0f);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(bloodbro_ms_state::get_tile_info)
{
	int const tile = m_tileram[Which][tile_index];
	tileinfo.set(Which, tile & 0xfff, tile >> 12, 0);
}

template <uint8_t Which>
void bloodbro_ms_state::tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_tileram[Which][offset]);
	m_tilemap[Which]->mark_tile_dirty(offset);
}

void bloodbro_ms_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const int NUM_SPRITES = 0x200;
	const int X_EXTRA_OFFSET = 240;

	for (int i = NUM_SPRITES - 2; i >= 0; i -= 2)
	{
		gfx_element *gfx = m_gfxdecode->gfx(3);

		uint16_t const attr0 = m_spriteram[i + 0];
		uint16_t const attr1 = m_spriteram[i + 1];

		uint16_t const attr2 = m_spriteram[i + NUM_SPRITES];
		//uint16_t const attr3 = m_spriteram[i + NUM_SPRITES + 1]; // unused?

		int ypos = attr0 & 0x00ff;
		int xpos = (attr1 & 0xff00) >> 8;
		xpos |= (attr2 & 0x8000) ? 0x100 : 0x000;

		ypos = (0xff - ypos);

		int tile = (attr0 & 0xff00) >> 8;
		tile |= (attr1 & 0x001f) << 8;

		int const flipx = (attr1 & 0x0020);
		int const flipy = (attr2 & 0x4000);

		// TODO: switch to prio_transpen
		gfx->transpen(bitmap, cliprect, tile, (attr2 & 0x0f00) >> 8, flipx, flipy, xpos - 16 - X_EXTRA_OFFSET, ypos - 16, 15);
	}
}

uint32_t bloodbro_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// TODO: is scrollx used?
	m_tilemap[1]->set_scrolly(0, (int8_t)m_scrollreg[1] + 1);

	bitmap.fill(0, cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);

	m_tilemap[2]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void bloodbro_ms_state::main_program_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x08bfff).ram(); // original vram is in here, but bootleg copies some of it elsewhere for use
	map(0x08c000, 0x08cfff).ram().w(FUNC(bloodbro_ms_state::tileram_w<0>)).share(m_tileram[0]);
	map(0x08d000, 0x08d7ff).ram();
	map(0x08d800, 0x08dfff).ram().w(FUNC(bloodbro_ms_state::tileram_w<2>)).share(m_tileram[2]);
	map(0x08e000, 0x08ffff).ram();
	map(0x0a0000, 0x0a000d).nopw(); // remnants of original code (Seibu sound system)?
	map(0x0c0100, 0x0c0101).nopw(); // ??? written once
	map(0x0e000e, 0x0e000e).w(m_soundlatch[1], FUNC(generic_latch_8_device::write));
	map(0x0e000f, 0x0e000f).w(m_soundlatch[0], FUNC(generic_latch_8_device::write));
	map(0x100000, 0x1003ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x100400, 0x1007ff).ram().w("palette", FUNC(palette_device::write16_ext)).share("palette_ext");
	map(0x100800, 0x100fff).ram();
	map(0x101000, 0x1017ff).ram().share(m_spriteram);
	map(0x101800, 0x101fff).ram();
	map(0x102000, 0x102001).nopw(); // ??? written often
	map(0x18d000, 0x18d7ff).w(FUNC(bloodbro_ms_state::tileram_w<1>)).share(m_tileram[1]);
	map(0x18d800, 0x18d803).lw16(NAME([this] (offs_t offset, uint16_t data) { m_scrollreg[offset] = data; }));
	map(0x0e0000, 0x0e0001).portr("DSW");
	map(0x0e0002, 0x0e0003).portr("IN0");
	map(0x0e0004, 0x0e0005).portr("IN1");
}

void bloodbro_ms_state::sound_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8000).w(FUNC(bloodbro_ms_state::adpcm_w));
	map(0x8000, 0xbfff).bankr(m_soundbank);
	map(0xc000, 0xc7ff).ram();
	map(0xd000, 0xd7ff).ram();
	map(0xdff0, 0xdff5).noprw(); // ???
	map(0xdffe, 0xdffe).r(m_soundlatch[1], FUNC(generic_latch_8_device::read)).w(FUNC(bloodbro_ms_state::soundlatch_w));
	map(0xdfff, 0xdfff).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));
	map(0xe000, 0xe003).w(FUNC(bloodbro_ms_state::ym_w));
	map(0xe008, 0xe009).r(m_ym[0], FUNC(ym2203_device::read));
	map(0xe00a, 0xe00b).r(m_ym[1], FUNC(ym2203_device::read));
}


static INPUT_PORTS_START( bloodbrom )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Mode" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	// Coin Mode 1
	PORT_DIPNAME( 0x001e, 0x001e, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:2,3,4,5") PORT_CONDITION("DSW", 0x0001, EQUALS, 0x0001)
	PORT_DIPSETTING(      0x0014, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0016, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x001a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x001e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0012, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	// Coin Mode 2
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:2,3") PORT_CONDITION("DSW", 0x0001, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5") PORT_CONDITION("DSW", 0x0001, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Starting Coin" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "x2" )

	PORT_DIPUNKNOWN_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "300K 500K+" )
	PORT_DIPSETTING(      0x0800, "500K+" )
	PORT_DIPSETTING(      0x0400, "500K" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tiles16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7, 512,513,514,515,516,517,518,519 },
	{ STEP16(0,32) },
	16 * 16 * 4
};


static GFXDECODE_START( gfx_bloodbro_ms )
	GFXDECODE_ENTRY( "gfx2", 0, tiles16x16x4_layout, 0x300, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles16x16x4_layout, 0x200, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_8x8x4_planar, 0x000, 16 )
	GFXDECODE_ENTRY( "sprites", 0, tiles16x16x4_layout, 0x100, 16 )
GFXDECODE_END


void bloodbro_ms_state::bloodbrom(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 22.1184_MHz_XTAL / 2); // divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &bloodbro_ms_state::main_program_map);
	m_maincpu->set_vblank_int("screen", FUNC(bloodbro_ms_state::irq4_line_hold));

	Z80(config, m_audiocpu, 24_MHz_XTAL / 8); // divisor unknown, no XTAL on the PCB, might also use the 20 MHz one
	m_audiocpu->set_addrmap(AS_PROGRAM, &bloodbro_ms_state::sound_program_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 240-1);
	screen.set_screen_update(FUNC(bloodbro_ms_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_444, 0x400);

	GFXDECODE(config, "gfxdecode", "palette", gfx_bloodbro_ms);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	GENERIC_LATCH_8(config, m_soundlatch[1]);

	YM2203(config, m_ym[0], 24_MHz_XTAL / 12).add_route(ALL_OUTPUTS, "mono", 0.75); // divisor unknown, no XTAL on the PCB, might also use the 20 MHz one

	YM2203(config, m_ym[1], 24_MHz_XTAL / 12).add_route(ALL_OUTPUTS, "mono", 0.75); // divisor unknown, no XTAL on the PCB, might also use the 20 MHz one

	MSM5205(config, m_msm, 24_MHz_XTAL / 64); // divisor unknown, no XTAL on the PCB, might also use the 20 MHz one
	m_msm->vck_legacy_callback().set(FUNC(bloodbro_ms_state::adpcm_int));
	m_msm->set_prescaler_selector(msm5205_device::S96_4B); // unverified
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.75);
}


ROM_START( bloodbrom )
	ROM_REGION( 0x100000, "maincpu", 0 ) // on MOD 6/1 board
	ROM_LOAD16_BYTE( "6-1_bb606.ic8",     0x00001, 0x10000, CRC(3c069061) SHA1(537a10376ad24537367fb221817789bdc31787fa) )
	ROM_LOAD16_BYTE( "6-1_bb603.ic17",    0x00000, 0x10000, CRC(10f4c8e9) SHA1(e5c078395b70b73d21c100c6b60cff89e4668473) )
	ROM_LOAD16_BYTE( "6-1_bb605.ic11",    0x20001, 0x10000, CRC(2dc3fb8c) SHA1(44e8e4136979464101385531f97cce27abe1de34) )
	ROM_LOAD16_BYTE( "6-1_bb602.ic20",    0x20000, 0x10000, CRC(8e507cce) SHA1(93bef8838cf8f73eb158dfe276f53c29f364fd45) )
	ROM_LOAD16_BYTE( "6-1_bb604.ic25",    0x40001, 0x10000, CRC(cc069a40) SHA1(314b27cde5427b285272840f41da097326b39ee9) )
	ROM_LOAD16_BYTE( "6-1_bb601.ic26",    0x40000, 0x10000, CRC(d06bf68d) SHA1(7df7a99805aa7dd2ad91fb3d641e369c058cc6ae) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on MOD 1/2 board
	ROM_LOAD( "1-2_bb101.ic12",  0x00000, 0x10000, CRC(3e184e74) SHA1(031cd37fe6d09daf8c9e88562da99fde03f52109) )

	// dumper's note: ROMs [bb4b1, bb4b2, bb4b3, bb4b4] and [bb4a1, bb4a2, bb4a3, bb4a4] have a strange setup
	// with pins 32, 31 and 30 soldered together and pin 2 connected between all four chips,
	// while the sockets are for 28 pin chips (with 27C512 silkscreened on the PCB behind the chips)
	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT ) // on one of the MOD 4/3 boards
	ROM_LOAD32_BYTE( "4-3-a_bb4a1.ic17",  0x00003, 0x20000, CRC(499c91db) SHA1(bd7142a311a4f3e606f8a31aafc0b504f3d5a2e4) )
	ROM_LOAD32_BYTE( "4-3-a_bb4a2.ic16",  0x00002, 0x20000, CRC(e8f87153) SHA1(f4147c971d1c66e7c6133c6318357ced7e30e217) )
	ROM_LOAD32_BYTE( "4-3-a_bb4a3.ic15",  0x00001, 0x20000, CRC(13b888f2) SHA1(7a53f78f22a09fe4db45c36bf3912ad379deca64) )
	ROM_LOAD32_BYTE( "4-3-a_bb4a4.ic14",  0x00000, 0x20000, CRC(19bc0508) SHA1(01c4eb570dc7ba9401085012d23bdb865df78029) )

	ROM_REGION( 0x80000, "gfx2", ROMREGION_INVERT ) // on another MOD 4/3 board
	ROM_LOAD32_BYTE( "4-3-b_bb4b1.ic17",  0x00003, 0x20000, CRC(aa86ae59) SHA1(c15a78eaaca36bebd3261cb2c4a2c232b967a135) )
	ROM_LOAD32_BYTE( "4-3-b_bb4b2.ic16",  0x00002, 0x20000, CRC(f25dd182) SHA1(eff29970c7b898744b08a151f9e17b68ce77e78d) )
	ROM_LOAD32_BYTE( "4-3-b_bb4b3.ic15",  0x00001, 0x20000, CRC(3efcb6aa) SHA1(0a162285d08e171e946147e0725db879643ae113) )
	ROM_LOAD32_BYTE( "4-3-b_bb4b4.ic14",  0x00000, 0x20000, CRC(6b5254fa) SHA1(1e9e3096e5f29554fb8f8cb0df0e5157f940f8c9) )

	// ROMs for frontmost tile layer (text)
	ROM_REGION( 0x20000, "gfx3", ROMREGION_INVERT ) // on another MOD 4/3 board
	ROM_LOAD( "4-3_bb401.ic17",    0x00000, 0x08000, CRC(07e12bd2) SHA1(33977f97f0c1a45055f6f8cb06294b2eb3c27acc) ) // 27256
	ROM_LOAD( "4-3_bb402.ic16",    0x08000, 0x08000, CRC(eca374ea) SHA1(4da5b876ccc9a7ac64f129ef18da521a56a022e0) ) // 27256
	ROM_LOAD( "4-3_bb403.ic15",    0x10000, 0x08000, CRC(d77b84d3) SHA1(baa7d3175e42c3872682ca3080e8d07ce3f5e43b) ) // 27256
	ROM_LOAD( "4-3_bb404.ic14",    0x18000, 0x08000, CRC(f8d2d4dc) SHA1(7210de80cb0939d9f703ed4c1e2d030dcb99d5f4) ) // 27256

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT ) // on MOD 51/1 board
	ROM_LOAD32_BYTE( "51-1-b_bb503.ic3",  0x00003, 0x10000, CRC(9d2a382d) SHA1(734b495ace73f07c622f64b305dafe43099395c1) )
	ROM_LOAD32_BYTE( "51-1-b_bb512.ic12", 0x00002, 0x10000, CRC(83bbb220) SHA1(8f43354c7cea89938d1115d7a0f27ede8f7d3e96) )
	ROM_LOAD32_BYTE( "51-1-b_bb518.ic18", 0x00001, 0x10000, CRC(efcf5b1d) SHA1(515b27f8e6df7ac7ed172cbd1ac64b14791de99f) )
	ROM_LOAD32_BYTE( "51-1-b_bb524.ic24", 0x00000, 0x10000, CRC(c4ccf38d) SHA1(be93ce6ed87c79fbd13838c0fe80526ce7e7e870) )
	ROM_LOAD32_BYTE( "51-1-b_bb504.ic4",  0x40003, 0x10000, CRC(1fc7f229) SHA1(37120c85a170f31bc4fbf287b1ba80bc319522ec) )
	ROM_LOAD32_BYTE( "51-1-b_bb513.ic13", 0x40002, 0x10000, CRC(3767456b) SHA1(3680807282079862cdfb5ec055e7d771e708545b) )
	ROM_LOAD32_BYTE( "51-1-b_bb519.ic19", 0x40001, 0x10000, CRC(77670244) SHA1(27a5572d86ae6e9a5ef076572a4b3a04a22c86e9) )
	ROM_LOAD32_BYTE( "51-1-b_bb525.ic25", 0x40000, 0x10000, CRC(25b4e119) SHA1(7e7d95aefee2b8d4dddf105c16d347ec65cd76a5) )
	ROM_LOAD32_BYTE( "51-1-b_bb505.ic5",  0x80003, 0x10000, CRC(3ec650ce) SHA1(28091f535fcd580f2d3a941251a9c4f662fcf2e4) )
	ROM_LOAD32_BYTE( "51-1-b_bb514.ic14", 0x80002, 0x10000, CRC(a29a2f44) SHA1(4e039d9a9b225179e84590d450eca3bed05bd3b8) )
	ROM_LOAD32_BYTE( "51-1-b_bb520.ic20", 0x80001, 0x10000, CRC(d7f3b09a) SHA1(339206a7c3389d4eac63e8314ba7fdda9de73be7) )
	ROM_LOAD32_BYTE( "51-1-b_bb526.ic26", 0x80000, 0x10000, CRC(1c2d70b0) SHA1(703f1acbcdaa7ff539f58829890d25b51a2e269e) )
	ROM_LOAD32_BYTE( "51-1-b_bb506.ic6",  0xc0003, 0x10000, CRC(10dba663) SHA1(ea0e4115ebb1c9f894c044a1eb11f135fcf5aba8) )
	ROM_LOAD32_BYTE( "51-1-b_bb515.ic15", 0xc0002, 0x10000, CRC(30110411) SHA1(fe9f418070c224d3a9acf6913bd4597b55afcc94) )
	ROM_LOAD32_BYTE( "51-1-b_bb521.ic21", 0xc0001, 0x10000, CRC(fb8cff4c) SHA1(5fa0b52140959e029911a28928b3efad4aa9f1db) )
	ROM_LOAD32_BYTE( "51-1-b_bb527.ic27", 0xc0000, 0x10000, CRC(a73cd7a5) SHA1(9106565d1c8a8e0efa8f5035106f3cdac2189107) )

	ROM_REGION( 0x0400, "proms", 0 )    // PROMs (function unknown)
	ROM_LOAD( "1-2_110_tbp18s030.ic20",  0x000, 0x020, CRC(e26e680a) SHA1(9bbe30e98e952a6113c64e1171330153ddf22ce7) )
	ROM_LOAD( "2_211_82s129.ic4",        0x100, 0x100, CRC(4f8c3e63) SHA1(0aa68fa1de6ca945027366a06752e834bbbc8d09) )
	ROM_LOAD( "2_202_82s129.ic12",       0x200, 0x100, CRC(e434128a) SHA1(ef0f6d8daef8b25211095577a182cdf120a272c1) )
	ROM_LOAD( "51-1_p0502_82s129n.ic10", 0x300, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )

	ROM_REGION( 0x1000, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "6-1_606_gal16v8-20hb1.ic13",    0x000, 0x117, NO_DUMP ) // Protected
	ROM_LOAD( "6-1_646_gal16v8-20hb1.ic7",     0x000, 0x117, NO_DUMP ) // Protected
	ROM_LOAD( "4-3_403_gal16v8-25hb1.ic29",    0x000, 0x117, NO_DUMP ) // Protected
	ROM_LOAD( "4-3-a_p0403_pal16r8acn.ic29",   0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) )
	ROM_LOAD( "4-3-b_403_gal16v8-25hb1.ic19",  0x000, 0x117, NO_DUMP ) // Protected
	ROM_LOAD( "51-1_503_gal16v8-25lp.ic48",    0x000, 0x117, NO_DUMP ) // Protected
	ROM_LOAD( "51-1-b_5146_gal16v8-20hb1.ic9", 0x000, 0x117, NO_DUMP ) // Protected
	ROM_LOAD( "51-1-b_5246_gal16v8-20hb1.ic8", 0x000, 0x117, NO_DUMP ) // Protected
ROM_END


// reorganize graphics into something we can decode with a single pass
void bloodbro_ms_state::descramble_16x16tiles(uint8_t *src, int len)
{
	std::vector<uint8_t> buffer(len);
	{
		for (int i = 0; i < len; i++)
		{
			int j = bitswap<20>(i, 19, 18, 17, 16, 15, 12, 11, 10, 9, 8, 7, 6, 5, 14, 13, 4, 3, 2, 1, 0);
			buffer[j] = src[i];
		}

		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}
}

void bloodbro_ms_state::init_bloodbrom()
{
	descramble_16x16tiles(memregion("gfx1")->base(), memregion("gfx1")->bytes());
	descramble_16x16tiles(memregion("gfx2")->base(), memregion("gfx2")->bytes());
}

} // anonymous namespace


GAME( 199?, bloodbrom,  bloodbro,  bloodbrom,  bloodbrom,  bloodbro_ms_state, init_bloodbrom, ROT0, "bootleg (Gaelco / Ervisa)", "Blood Bros. (Modular System)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
