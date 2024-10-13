// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
/*
    Diet Go Go

    Driver by Bryan McPhail and David Haywood.

Hold both START buttons on bootup to display version notice.

Diet Go Go (Japan)   DATA EAST
DE-0370-2

NAME    LOCATION   TYPE
-----------------------
JW-02    14M       27C512
JW-01-2  5H        27C2001
JW-00-2  4H         "
PAL16L8B 7H
PAL16L8B 6H
PAL16R6A 11H

European versions were seen with either the MAY-01 and MAY-02,
or MAY-04 and MAY-05 sprite ROMs. The latter is more common on newer versions,
these contain data for alternative title screen graphics enabled with a DIP switch.

*/

#include "emu.h"

#include "cpu/h6280/h6280.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

#include "decocrpt.h"
#include "deco102.h"
#include "deco104.h"
#include "deco16ic.h"
#include "decospr.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class dietgo_state : public driver_device
{
public:
	dietgo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_pf_rowscroll(*this, "pf%u_rowscroll", 1)
		, m_spriteram(*this, "spriteram")
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_deco_tilegen(*this, "tilegen")
		, m_deco104(*this, "ioprot104")
		, m_sprgen(*this, "spritegen")
	{ }

	void dietgo(machine_config &config);

	void init_dietgo();

private:
	// memory pointers
	required_shared_ptr_array<uint16_t, 2> m_pf_rowscroll;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_decrypted_opcodes;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device<deco16ic_device> m_deco_tilegen;
	required_device<deco104_device> m_deco104;
	required_device<decospr_device> m_sprgen;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECO16IC_BANK_CB_MEMBER(bank_callback);

	uint16_t ioprot_r(offs_t offset);
	void ioprot_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


uint32_t dietgo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t const flip = m_deco_tilegen->pf_control_r(0);

	flip_screen_set(BIT(flip, 7));
	m_sprgen->set_flip_screen(BIT(flip, 7));
	m_deco_tilegen->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);

	bitmap.fill(256, cliprect); // not verified

	m_deco_tilegen->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_deco_tilegen->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);

	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	return 0;
}

uint16_t dietgo_state::ioprot_r(offs_t offset)
{
	int const real_address = 0 + (offset * 2);
	int const deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	uint16_t const data = m_deco104->read_data(deco146_addr, cs);
	return data;
}

void dietgo_state::ioprot_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int const real_address = 0 + (offset * 2);
	int const deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	m_deco104->write_data(deco146_addr, data, mem_mask, cs);
}


void dietgo_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x20000f).w(m_deco_tilegen, FUNC(deco16ic_device::pf_control_w));
	map(0x210000, 0x211fff).w(m_deco_tilegen, FUNC(deco16ic_device::pf1_data_w));
	map(0x212000, 0x213fff).w(m_deco_tilegen, FUNC(deco16ic_device::pf2_data_w));
	map(0x220000, 0x2207ff).writeonly().share(m_pf_rowscroll[0]);
	map(0x222000, 0x2227ff).writeonly().share(m_pf_rowscroll[1]);
	map(0x280000, 0x2807ff).ram().share(m_spriteram);
	map(0x300000, 0x300bff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x340000, 0x343fff).rw(FUNC(dietgo_state::ioprot_r), FUNC(dietgo_state::ioprot_w)).share("prot16ram"); // Protection device
	map(0x380000, 0x38ffff).ram();
}

void dietgo_state::decrypted_opcodes_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().share(m_decrypted_opcodes);
}


// Physical memory map (21 bits)
void dietgo_state::sound_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x100000, 0x100001).noprw();     // YM2203 - this board doesn't have one
	map(0x110000, 0x110001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x120000, 0x120001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x130000, 0x130001).noprw();     // This board only has 1 oki chip
	map(0x140000, 0x140000).r(m_deco104, FUNC(deco104_device::soundlatch_r));
	map(0x1f0000, 0x1f1fff).ram();
}



static INPUT_PORTS_START( dietgo )
	PORT_START("SYSTEM")   // Verified as 4 bit input port only
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Continue Coin" )
	PORT_DIPSETTING(      0x0080, "1 Start/1 Continue" )
	PORT_DIPSETTING(      0x0000, "2 Start/1 Continue" )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) ) // Demo_Sounds
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) ) // Players don't move in attract mode if on!?
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dietgon ) // New European version with optional alternative graphics
	PORT_INCLUDE( dietgo )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x2000, 0x2000, "Alternative graphics" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
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
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	32*16
};

static GFXDECODE_START( gfx_dietgo )
	GFXDECODE_ENTRY( "tiles",   0, tile_8x8_layout,     0, 32 )    // Tiles (8x8)
	GFXDECODE_ENTRY( "tiles",   0, tile_16x16_layout,   0, 32 )    // Tiles (16x16)
GFXDECODE_END

static GFXDECODE_START( gfx_dietgo_spr )
	GFXDECODE_ENTRY( "sprites", 0, tile_16x16_layout, 512, 16 )    // Sprites (16x16)
GFXDECODE_END

DECO16IC_BANK_CB_MEMBER(dietgo_state::bank_callback)
{
	return (bank & 0x70) << 8;
}

void dietgo_state::dietgo(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 28_MHz_XTAL / 2); // DE102 (verified on PCB)
	m_maincpu->set_addrmap(AS_PROGRAM, &dietgo_state::main_map);
	m_maincpu->set_addrmap(AS_OPCODES, &dietgo_state::decrypted_opcodes_map);
	m_maincpu->set_vblank_int("screen", FUNC(dietgo_state::irq6_line_hold));

	H6280(config, m_audiocpu, 32.22_MHz_XTAL / 4 / 3); // Custom chip 45; XIN is 32.220MHZ/4, verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &dietgo_state::sound_map);
	m_audiocpu->add_route(ALL_OUTPUTS, "mono", 0); // internal sound unused

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(dietgo_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_888, 1024);

	GFXDECODE(config, "gfxdecode", "palette", gfx_dietgo);

	DECO16IC(config, m_deco_tilegen, 0);
	m_deco_tilegen->set_pf1_size(DECO_64x32);
	m_deco_tilegen->set_pf2_size(DECO_64x32);
	m_deco_tilegen->set_pf1_col_bank(0x00);
	m_deco_tilegen->set_pf2_col_bank(0x10);
	m_deco_tilegen->set_pf1_col_mask(0x0f);
	m_deco_tilegen->set_pf2_col_mask(0x0f);
	m_deco_tilegen->set_bank1_callback(FUNC(dietgo_state::bank_callback));
	m_deco_tilegen->set_bank2_callback(FUNC(dietgo_state::bank_callback));
	m_deco_tilegen->set_pf12_8x8_bank(0);
	m_deco_tilegen->set_pf12_16x16_bank(1);
	m_deco_tilegen->set_gfxdecode_tag("gfxdecode");

	DECO_SPRITE(config, m_sprgen, 0, "palette", gfx_dietgo_spr);

	DECO104PROT(config, m_deco104, 0);
	m_deco104->port_a_cb().set_ioport("INPUTS");
	m_deco104->port_b_cb().set_ioport("SYSTEM");
	m_deco104->port_c_cb().set_ioport("DSW");
	m_deco104->soundlatch_irq_cb().set_inputline(m_audiocpu, 0);
	m_deco104->set_interface_scramble_interleave();
	m_deco104->set_use_magic_read_address_xor(true);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 32.22_MHz_XTAL / 9)); // verified on PCB
	ymsnd.irq_handler().set_inputline(m_audiocpu, 1); // IRQ2
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.45);

	OKIM6295(config, "oki", 32.22_MHz_XTAL / 32, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.60); // verified on PCB
}

ROM_START( dietgo ) // same version 1.1 and same program ROMs as dietgoe but newer sprite ROMs
	ROM_REGION( 0x80000, "maincpu", 0 ) // DE102 code (encrypted)
	ROM_LOAD16_BYTE( "jy_00-3.4h", 0x000001, 0x040000, CRC(a863ad0c) SHA1(61bf2fe5dce92e3995791a7e9ef813d64bcc2b93) )
	ROM_LOAD16_BYTE( "jy_01-3.5h", 0x000000, 0x040000, CRC(ef243eda) SHA1(b8efbb80c5bf40ef6c26a06fc7232d6e63596cb4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "jy_02.14m", 0x00000, 0x10000, CRC(4e3492a5) SHA1(5f302bdbacbf95ea9f3694c48545a1d6bba4b019) )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "may-00.10a", 0x00000, 0x100000, CRC(234d1f8d) SHA1(42d23aad20df20cbd2359cc12bdd47636b2027d3) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "may-04_w78_9235kd011.14a", 0x000000, 0x100000, CRC(dedd2dd3) SHA1(c1021edb0b377a030ab9593c838083f0f3b996b2) )
	ROM_LOAD( "may-05_w79_9235kd019.16a", 0x100000, 0x100000, CRC(cb23835f) SHA1(c504ff99f9029355f69e7fd7e9528d647bd491bf) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "may-03.11l", 0x00000, 0x80000, CRC(b6e42bae) SHA1(c282cdf7db30fb63340cc609bf00f5ab63a75583) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8b_vd-00.6h",  0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b_vd-01.7h",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r6a_vd-02.11h", 0x0400, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( dietgoe ) // same version 1.1 and same date as dietgoea but newer version in ROM labels
	ROM_REGION( 0x80000, "maincpu", 0 ) // DE102 code (encrypted)
	ROM_LOAD16_BYTE( "jy_00-3.4h", 0x000001, 0x040000, CRC(a863ad0c) SHA1(61bf2fe5dce92e3995791a7e9ef813d64bcc2b93) )
	ROM_LOAD16_BYTE( "jy_01-3.5h", 0x000000, 0x040000, CRC(ef243eda) SHA1(b8efbb80c5bf40ef6c26a06fc7232d6e63596cb4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "jy_02.14m", 0x00000, 0x10000, CRC(4e3492a5) SHA1(5f302bdbacbf95ea9f3694c48545a1d6bba4b019) )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "may-00.10a", 0x00000, 0x100000, CRC(234d1f8d) SHA1(42d23aad20df20cbd2359cc12bdd47636b2027d3) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "may-01.14a", 0x000000, 0x100000, CRC(2da57d04) SHA1(3898e9fef365ecaa4d86aa11756b527a4fffb494) )
	ROM_LOAD( "may-02.16a", 0x100000, 0x100000, CRC(3a66a713) SHA1(beeb99156332cf4870738f7769b719a02d7b40af) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "may-03.11l", 0x00000, 0x80000, CRC(b6e42bae) SHA1(c282cdf7db30fb63340cc609bf00f5ab63a75583) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8b_vd-00.6h",  0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b_vd-01.7h",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r6a_vd-02.11h", 0x0400, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( dietgoea ) // weird, still version 1.1 and same date
	ROM_REGION( 0x80000, "maincpu", 0 ) // DE102 code (encrypted)
	ROM_LOAD16_BYTE( "jy_00-2.4h", 0x000001, 0x040000, CRC(014dcf62) SHA1(1a28ce4a643ec8b6f062b1200342ed4dc6db38a1) )
	ROM_LOAD16_BYTE( "jy_01-2.5h", 0x000000, 0x040000, CRC(793ebd83) SHA1(b9178f18ce6e9fca848cbbf9dce3f3856672bf94) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "jy_02.14m", 0x00000, 0x10000, CRC(4e3492a5) SHA1(5f302bdbacbf95ea9f3694c48545a1d6bba4b019) )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "may-00.10a", 0x00000, 0x100000, CRC(234d1f8d) SHA1(42d23aad20df20cbd2359cc12bdd47636b2027d3) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "may-01.14a", 0x000000, 0x100000, CRC(2da57d04) SHA1(3898e9fef365ecaa4d86aa11756b527a4fffb494) )
	ROM_LOAD( "may-02.16a", 0x100000, 0x100000, CRC(3a66a713) SHA1(beeb99156332cf4870738f7769b719a02d7b40af) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "may-03.11l", 0x00000, 0x80000, CRC(b6e42bae) SHA1(c282cdf7db30fb63340cc609bf00f5ab63a75583) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8b_vd-00.6h",  0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b_vd-01.7h",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r6a_vd-02.11h", 0x0400, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( dietgoeb ) // weird, still version 1.1 but different (earlier) date
	ROM_REGION( 0x80000, "maincpu", 0 ) // DE102 code (encrypted)
	ROM_LOAD16_BYTE( "jy_00-1.4h", 0x000001, 0x040000, CRC(8bce137d) SHA1(55f5b1c89330803c6147f9656f2cabe8d1de8478) )
	ROM_LOAD16_BYTE( "jy_01-1.5h", 0x000000, 0x040000, CRC(eca50450) SHA1(1a24117e3b1b66d7dbc5484c94cc2c627d34e6a3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "jy_02.14m", 0x00000, 0x10000, CRC(4e3492a5) SHA1(5f302bdbacbf95ea9f3694c48545a1d6bba4b019) )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "may-00.10a", 0x00000, 0x100000, CRC(234d1f8d) SHA1(42d23aad20df20cbd2359cc12bdd47636b2027d3) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "may-01.14a", 0x000000, 0x100000, CRC(2da57d04) SHA1(3898e9fef365ecaa4d86aa11756b527a4fffb494) )
	ROM_LOAD( "may-02.16a", 0x100000, 0x100000, CRC(3a66a713) SHA1(beeb99156332cf4870738f7769b719a02d7b40af) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "may-03.11l", 0x00000, 0x80000, CRC(b6e42bae) SHA1(c282cdf7db30fb63340cc609bf00f5ab63a75583) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8b_vd-00.6h",  0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b_vd-01.7h",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r6a_vd-02.11h", 0x0400, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( dietgou )
	ROM_REGION( 0x80000, "maincpu", 0 ) // DE102 code (encrypted)
	ROM_LOAD16_BYTE( "jx_00-.4h", 0x000001, 0x040000, CRC(1a9de04f) SHA1(7ce1e7cf4cdce2b02da4df2a6ae9a9e665e24422) )
	ROM_LOAD16_BYTE( "jx_01-.5h", 0x000000, 0x040000, CRC(79c097c8) SHA1(be49055ee324535e1118d243bd49e74ec1d2a2d7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "jx_02.14m", 0x00000, 0x10000, CRC(4e3492a5) SHA1(5f302bdbacbf95ea9f3694c48545a1d6bba4b019) ) // Same as other regions but different label

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "may-00.10a", 0x00000, 0x100000, CRC(234d1f8d) SHA1(42d23aad20df20cbd2359cc12bdd47636b2027d3) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "may-01.14a", 0x000000, 0x100000, CRC(2da57d04) SHA1(3898e9fef365ecaa4d86aa11756b527a4fffb494) )
	ROM_LOAD( "may-02.16a", 0x100000, 0x100000, CRC(3a66a713) SHA1(beeb99156332cf4870738f7769b719a02d7b40af) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "may-03.11l", 0x00000, 0x80000, CRC(b6e42bae) SHA1(c282cdf7db30fb63340cc609bf00f5ab63a75583) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8b_vd-00.6h",  0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b_vd-01.7h",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r6a_vd-02.11h", 0x0400, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( dietgoj )
	ROM_REGION( 0x80000, "maincpu", 0 ) // DE102 code (encrypted)
	ROM_LOAD16_BYTE( "jw_00-2.4h", 0x000001, 0x040000, CRC(e6ba6c49) SHA1(d5eaea81f1353c58c03faae67428f7ee98e766b1) )
	ROM_LOAD16_BYTE( "jw_01-2.5h", 0x000000, 0x040000, CRC(684a3d57) SHA1(bd7a57ba837a1dc8f92b5ebcb46e50db1f98524f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "jw_02.14m", 0x00000, 0x10000, CRC(4e3492a5) SHA1(5f302bdbacbf95ea9f3694c48545a1d6bba4b019) ) // Same as other regions but different label

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "may-00.10a", 0x00000, 0x100000, CRC(234d1f8d) SHA1(42d23aad20df20cbd2359cc12bdd47636b2027d3) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "may-01.14a", 0x000000, 0x100000, CRC(2da57d04) SHA1(3898e9fef365ecaa4d86aa11756b527a4fffb494) )
	ROM_LOAD( "may-02.16a", 0x100000, 0x100000, CRC(3a66a713) SHA1(beeb99156332cf4870738f7769b719a02d7b40af) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "may-03.11l", 0x00000, 0x80000, CRC(b6e42bae) SHA1(c282cdf7db30fb63340cc609bf00f5ab63a75583) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8b_vd-00.6h",  0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b_vd-01.7h",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r6a_vd-02.11h", 0x0400, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END


void dietgo_state::init_dietgo()
{
	deco56_decrypt_gfx(machine(), "tiles");
	deco102_decrypt_cpu((uint16_t *)memregion("maincpu")->base(), m_decrypted_opcodes, 0x80000, 0xe9ba, 0x01, 0x19);
}

} // Anonymous namespace


GAME( 1992, dietgo,   0,      dietgo, dietgon, dietgo_state, init_dietgo, ROT0, "Data East Corporation", "Diet Go Go (Europe v1.1 1992.09.26, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, dietgoe,  dietgo, dietgo, dietgo,  dietgo_state, init_dietgo, ROT0, "Data East Corporation", "Diet Go Go (Europe v1.1 1992.09.26, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, dietgoea, dietgo, dietgo, dietgo,  dietgo_state, init_dietgo, ROT0, "Data East Corporation", "Diet Go Go (Europe v1.1 1992.09.26, set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, dietgoeb, dietgo, dietgo, dietgo,  dietgo_state, init_dietgo, ROT0, "Data East Corporation", "Diet Go Go (Europe v1.1 1992.08.04)",        MACHINE_SUPPORTS_SAVE )
GAME( 1992, dietgou,  dietgo, dietgo, dietgo,  dietgo_state, init_dietgo, ROT0, "Data East Corporation", "Diet Go Go (USA v1.1 1992.09.26)",           MACHINE_SUPPORTS_SAVE )
GAME( 1992, dietgoj,  dietgo, dietgo, dietgo,  dietgo_state, init_dietgo, ROT0, "Data East Corporation", "Diet Go Go (Japan v1.1 1992.09.26)",         MACHINE_SUPPORTS_SAVE )
