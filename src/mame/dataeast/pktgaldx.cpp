// license:BSD-3-Clause
// copyright-holders:David Haywood, Bryan McPhail
/*

Pocket Gal Deluxe
Nihon System Inc., 1993

This game runs on Data East hardware.

PCB Layout
----------

DEC-22V0  DE-0378-2
|-------------------------------------|
|  M6295(2)  KG01.14F     DE102  62256|
|  M6295(1)  MAZ-03.13F          62256|
|                                     |
|   32.220MHz              KG00-2.12A |
|                                     |
|J           DE56              DE71   |
|A                                    |
|M     DE153            28MHz         |
|M                    6264     DE52   |
|A                    6264            |
|                                     |
|                                     |
|                                     |
|DSW1                   MAZ-01.3B     |
|     DE104  MAZ-02.2H                |
|DSW2                   MAZ-00.1B     |
|-------------------------------------|

Notes:
      - CPU is DE102. The clock input is 14.000MHz on pin 6
        It's a custom-made encrypted 68000.
        The package is a Quad Flat Pack, has 128 pins and is symmetrically square (1 1/4" square from tip to tip).
      - M6295(1) clock: 2.01375MHz (32.220 / 16)
        M6295(2) clock: 1.006875MHz (32.220 / 32)
      - VSync: 58Hz


    Driver by David Haywood and Bryan McPhail

NOTE: Hold down both Player1 & Player2 Start buttons during boot up to see version & region

*/

/*
original TODO:
    verify sound.

bootleg TODO:

    Fix GFX glitches in background of girls after each level.
    Tidy up code.

*/

#include "emu.h"

#include "deco16ic.h"
#include "deco102.h"
#include "deco104.h"
#include "decocrpt.h"
#include "decospr.h"

#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class base_state : public driver_device
{
public:
	base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki2(*this, "oki2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

protected:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void oki_bank_w(uint16_t data);

	void vblank_w(int state);
	void vblank_ack_w(uint16_t data);
};

class pktgaldx_state : public base_state
{
public:
	pktgaldx_state(const machine_config &mconfig, device_type type, const char *tag) :
		base_state(mconfig, type, tag),
		m_pf_rowscroll(*this, "pf%u_rowscroll", 1U),
		m_spriteram(*this, "spriteram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_deco104(*this, "ioprot104"),
		m_sprgen(*this, "spritegen"),
		m_deco_tilegen(*this, "tilegen")
	{ }

	void pktgaldx(machine_config &config);

	void driver_init();

private:
	// memory pointers
	required_shared_ptr_array<uint16_t, 2> m_pf_rowscroll;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_decrypted_opcodes;

	// devices
	required_device<deco104_device> m_deco104;
	required_device<decospr_device> m_sprgen;
	required_device<deco16ic_device> m_deco_tilegen;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECO16IC_BANK_CB_MEMBER(bank_callback);

	uint16_t ioprot_r(offs_t offset);
	void ioprot_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void decrypted_opcodes_map(address_map &map);
	void prg_map(address_map &map);
};

class pktgaldxb_state : public base_state
{
public:
	pktgaldxb_state(const machine_config &mconfig, device_type type, const char *tag) :
		base_state(mconfig, type, tag),
		m_fgram(*this, "fgram"),
		m_sprites(*this, "sprites")
	{ }

	void pktgaldxb(machine_config &config);

private:
	// memory pointers
	required_shared_ptr<uint16_t> m_fgram;
	required_shared_ptr<uint16_t> m_sprites;

	uint16_t unknown_r();
	uint16_t protection_r();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map);
};


uint32_t pktgaldx_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t const flip = m_deco_tilegen->pf_control_r(0);

	// sprites are flipped relative to tilemaps
	flip_screen_set(BIT(flip, 7));
	m_sprgen->set_flip_screen(!BIT(flip, 7));
	m_deco_tilegen->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);

	bitmap.fill(0, cliprect); // not confirmed
	screen.priority().fill(0);

	m_deco_tilegen->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	m_deco_tilegen->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

uint32_t pktgaldxb_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	// the bootleg seems to treat the tilemaps as sprites
	for (int offset = 0; offset < 0x1600 / 2; offset += 8)
	{
		int const tileno = m_sprites[offset + 3] | (m_sprites[offset + 2] << 16);
		int const colour = m_sprites[offset + 1] >> 1;
		int x = m_sprites[offset + 0];
		int y = m_sprites[offset + 4];

		x -= 0xc2;
		y &= 0x1ff;
		y -= 8;

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, tileno ^ 0x1000, colour, 0, 0, x, y, 0);
	}

	for (int offset = 0x1600/2; offset < 0x2000 / 2; offset += 8)
	{
		int const tileno = m_sprites[offset + 3] | (m_sprites[offset + 2] << 16);
		int const colour = m_sprites[offset + 1] >> 1;
		int x = m_sprites[offset + 0] & 0x1ff;
		int y = m_sprites[offset + 4] & 0x0ff;

		x -= 0xc2;
		y &= 0x1ff;
		y -= 8;

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, tileno ^ 0x4000, colour, 0, 0, x, y, 0);
	}

	for (int offset = 0x2000/2; offset < 0x4000 / 2; offset += 8)
	{
		int const tileno = m_sprites[offset + 3] | (m_sprites[offset + 2] << 16);
		int const colour = m_sprites[offset + 1] >> 1;
		int x = m_sprites[offset + 0] & 0x1ff;
		int y = m_sprites[offset + 4] & 0x0ff;

		x -= 0xc2;
		y &= 0x1ff;
		y -= 8;

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, tileno ^ 0x3000, colour, 0, 0, x, y, 0);
	}

	return 0;
}


/**********************************************************************************/

void base_state::oki_bank_w(uint16_t data)
{
	m_oki2->set_rom_bank(data & 3);
}

/**********************************************************************************/

uint16_t pktgaldx_state::ioprot_r(offs_t offset)
{
	int const real_address = 0 + (offset * 2);
	uint8_t cs = 0;
	uint16_t const data = m_deco104->read_data(real_address & 0x7fff, cs);
	return data;
}

void pktgaldx_state::ioprot_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int const real_address = 0 + (offset * 2);
	uint8_t cs = 0;
	m_deco104->write_data(real_address & 0x7fff, data, mem_mask, cs);
}

void base_state::vblank_w(int state)
{
	if (state)
		m_maincpu->set_input_line(6, ASSERT_LINE);
}

void base_state::vblank_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(6, CLEAR_LINE);
}

void pktgaldx_state::prg_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

	map(0x100000, 0x100fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x102000, 0x102fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x110000, 0x1107ff).ram().share(m_pf_rowscroll[0]);
	map(0x112000, 0x1127ff).ram().share(m_pf_rowscroll[1]);

	map(0x120000, 0x1207ff).ram().share(m_spriteram);
	map(0x130000, 0x130fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0x140000, 0x14000f).w("oki1", FUNC(okim6295_device::write)).umask16(0x00ff);
	map(0x140007, 0x140007).r("oki1", FUNC(okim6295_device::read));
	map(0x150000, 0x15000f).w(m_oki2, FUNC(okim6295_device::write)).umask16(0x00ff);
	map(0x150007, 0x150007).r(m_oki2, FUNC(okim6295_device::read));

	map(0x161800, 0x16180f).w(m_deco_tilegen, FUNC(deco16ic_device::pf_control_w));
	map(0x164800, 0x164801).w(FUNC(pktgaldx_state::oki_bank_w));
	map(0x166800, 0x166801).w(FUNC(pktgaldx_state::vblank_ack_w));
	map(0x167800, 0x167fff).rw(FUNC(pktgaldx_state::ioprot_r), FUNC(pktgaldx_state::ioprot_w)).share("prot16ram");

	map(0x170000, 0x17ffff).ram();
}

void pktgaldx_state::decrypted_opcodes_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().share(m_decrypted_opcodes);
}


// Pocket Gal Deluxe (bootleg!)

uint16_t pktgaldxb_state::unknown_r()
{
	return 0xffff;
}

uint16_t pktgaldxb_state::protection_r()
{
	if (!machine().side_effects_disabled())
		logerror("protection_r address %06x\n", m_maincpu->pc());
	return -1;
}

/*
cpu #0 (PC=0000A0DE): unmapped program memory word read from 00167DB2 & FFFF
cpu #0 (PC=00009DFA): unmapped program memory word read from 00167C4C & FFFF
cpu #0 (PC=00009E58): unmapped program memory word read from 00167842 & 00FF
cpu #0 (PC=00009E80): unmapped program memory word read from 00167842 & FF00
cpu #0 (PC=00009AFE): unmapped program memory word read from 00167842 & 00FF
cpu #0 (PC=00009B12): unmapped program memory word read from 00167842 & FF00
cpu #0 (PC=0000923C): unmapped program memory word read from 00167DB2 & 00FF
*/

// do the 300000 addresses somehow interact with the protection addresses on this bootleg?
// or maybe protection writes go to sound ...

void pktgaldxb_state::prg_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x100fff).ram().share(m_fgram); // fgram on original?
	map(0x102000, 0x102fff).ram(); // bgram on original?
	map(0x120000, 0x123fff).ram().share(m_sprites);

	map(0x130000, 0x130fff).ram(); // palette on original?

	map(0x140000, 0x14000f).w("oki1", FUNC(okim6295_device::write)).umask16(0x00ff);
	map(0x140007, 0x140007).r("oki1", FUNC(okim6295_device::read));
	map(0x150000, 0x15000f).w(m_oki2, FUNC(okim6295_device::write)).umask16(0x00ff);
	map(0x150007, 0x150007).r(m_oki2, FUNC(okim6295_device::read));

//  map(0x160000, 0x167fff).ram();
	map(0x164800, 0x164801).w(FUNC(pktgaldxb_state::oki_bank_w));
	map(0x16500a, 0x16500b).r(FUNC(pktgaldxb_state::unknown_r));
	map(0x166800, 0x166801).w(FUNC(pktgaldxb_state::vblank_ack_w));
	/* should we really be using these to read the i/o in the BOOTLEG?
	  these look like i/o through protection ... */
	map(0x167842, 0x167843).portr("INPUTS");
	map(0x167c4c, 0x167c4d).portr("DSW");
	map(0x167db2, 0x167db3).portr("SYSTEM");

	map(0x167d10, 0x167d11).r(FUNC(pktgaldxb_state::protection_r)); // check code at 6ea
	map(0x167d1a, 0x167d1b).r(FUNC(pktgaldxb_state::protection_r)); // check code at 7c4

	map(0x170000, 0x17ffff).ram();

	map(0x300000, 0x30000f).ram(); // ??

	map(0x330000, 0x330bff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); // extra colours?
}


/**********************************************************************************/

static INPUT_PORTS_START( pktgaldx )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

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
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0300, "4" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Time"  )                 PORT_DIPLOCATION("SW2:3,4") // Listed as "Difficulty"
	PORT_DIPSETTING(      0x0000, "60" )
	PORT_DIPSETTING(      0x0400, "80" )
	PORT_DIPSETTING(      0x0c00, "100" )
	PORT_DIPSETTING(      0x0800, "120" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Free_Play ) )            PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout tile_8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*2) },
	8*16
};

static const gfx_layout tile_16x16_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ STEP8(8*2*16, 1),STEP8(0, 1) },
	{ STEP16(0, 8*2) },
	32*16
};

static GFXDECODE_START( gfx_pktgaldx )
	GFXDECODE_ENTRY( "tiles",   0, tile_8x8_layout,     0, 32 )    // 8x8
	GFXDECODE_ENTRY( "tiles",   0, tile_16x16_layout,   0, 32 )    // 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_pktgaldx_spr )
	GFXDECODE_ENTRY( "sprites", 0, tile_16x16_layout, 512, 32 )    // 16x16
GFXDECODE_END

static const gfx_layout bootleg_spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 48,52,56,60,32,36,40,44,16,20,24,28,0,4,8,12 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64, 9*64,10*64,11*64,12*64,13*64,14*64,15*64},
	16*64
};

static GFXDECODE_START( gfx_bootleg )
	GFXDECODE_ENTRY( "tiles", 0, bootleg_spritelayout,     0, 64 )
GFXDECODE_END


DECO16IC_BANK_CB_MEMBER(pktgaldx_state::bank_callback)
{
	return ((bank >> 4) & 0x7) * 0x1000;
}


void pktgaldx_state::pktgaldx(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 28_MHz_XTAL / 2); // The clock input is 14.000MHz on pin 6
	m_maincpu->set_addrmap(AS_PROGRAM, &pktgaldx_state::prg_map);
	m_maincpu->set_addrmap(AS_OPCODES, &pktgaldx_state::decrypted_opcodes_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(pktgaldx_state::screen_update));
	screen.screen_vblank().set(FUNC(pktgaldx_state::vblank_w));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 4096/4);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pktgaldx);

	DECO16IC(config, m_deco_tilegen, 0);
	m_deco_tilegen->set_pf1_size(DECO_64x32);
	m_deco_tilegen->set_pf2_size(DECO_64x32);
	m_deco_tilegen->set_pf1_col_bank(0x00);
	m_deco_tilegen->set_pf2_col_bank(0x10);
	m_deco_tilegen->set_pf1_col_mask(0x0f);
	m_deco_tilegen->set_pf2_col_mask(0x0f);
	m_deco_tilegen->set_bank1_callback(FUNC(pktgaldx_state::bank_callback));
	m_deco_tilegen->set_bank2_callback(FUNC(pktgaldx_state::bank_callback));
	m_deco_tilegen->set_pf12_8x8_bank(0);
	m_deco_tilegen->set_pf12_16x16_bank(1);
	m_deco_tilegen->set_gfxdecode_tag(m_gfxdecode);

	DECO_SPRITE(config, m_sprgen, 0, m_palette, gfx_pktgaldx_spr);

	DECO104PROT(config, m_deco104, 0);
	m_deco104->port_a_cb().set_ioport("INPUTS");
	m_deco104->port_b_cb().set_ioport("SYSTEM");
	m_deco104->port_c_cb().set_ioport("DSW");
	m_deco104->set_interface_scramble(8,9,  4,5,6,7,    1,0,3,2); // hopefully this is correct, nothing else uses this arrangement!

	// sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki1(OKIM6295(config, "oki1", 32.22_MHz_XTAL / 32, okim6295_device::PIN7_HIGH));
	oki1.add_route(ALL_OUTPUTS, "mono", 0.75);

	OKIM6295(config, m_oki2, 32.22_MHz_XTAL / 16, okim6295_device::PIN7_HIGH);
	m_oki2->add_route(ALL_OUTPUTS, "mono", 0.60);
}


void pktgaldxb_state::pktgaldxb(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pktgaldxb_state::prg_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(pktgaldxb_state::screen_update));
	screen.screen_vblank().set(FUNC(pktgaldxb_state::vblank_w));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 0xc00/4);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bootleg);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki1(OKIM6295(config, "oki1", 32220000 / 32, okim6295_device::PIN7_HIGH));
	oki1.add_route(ALL_OUTPUTS, "mono", 0.75);

	OKIM6295(config, m_oki2, 32220000 / 16, okim6295_device::PIN7_HIGH);
	m_oki2->add_route(ALL_OUTPUTS, "mono", 0.60);
}


ROM_START( pktgaldx )
	ROM_REGION( 0x80000, "maincpu", 0 ) // DE102 code (encrypted)
	ROM_LOAD16_WORD_SWAP( "ke00-2.12a",    0x00000, 0x80000, CRC(b04baf3a) SHA1(680d1b4ab4b6edef36cd96a60539fb7c2dac9637) ) // Version 3.00 Euro

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "maz-02.2h", 0x00000, 0x100000, CRC(c9d35a59) SHA1(07b44c7d7d76b668b4d6ca5672bd1c2910228e68) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "maz-00.1b", 0x000000, 0x080000, CRC(fa3071f4) SHA1(72e7d920e9ca94f8cb166007a9e9e5426a201af8) )
	ROM_LOAD( "maz-01.3b", 0x080000, 0x080000, CRC(4934fe21) SHA1(b852249f59906d69d32160ebaf9b4781193227e4) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "ke01.14f", 0x00000, 0x20000, CRC(8a106263) SHA1(229ab17403c2b8f4e89a90a8cda2f3c3a4b55d9e) )

	ROM_REGION( 0x100000, "oki2", 0 ) // banked
	ROM_LOAD( "maz-03.13f", 0x00000, 0x100000, CRC(a313c964) SHA1(4a3664c4e2c44a017a0ab6a6d4361799cbda57b5) )
ROM_END

ROM_START( pktgaldxj )
	ROM_REGION( 0x80000, "maincpu", 0 ) // DE102 code (encrypted)
	ROM_LOAD16_WORD_SWAP( "kg00-2.12a",    0x00000, 0x80000, CRC(62dc4137) SHA1(23887dc3f6e7c4cdcb1bf4f4c87fe3cbe8cdbe69) ) // Version 3.00 Japan

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "maz-02.2h", 0x00000, 0x100000, CRC(c9d35a59) SHA1(07b44c7d7d76b668b4d6ca5672bd1c2910228e68) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "maz-00.1b", 0x000000, 0x080000, CRC(fa3071f4) SHA1(72e7d920e9ca94f8cb166007a9e9e5426a201af8) )
	ROM_LOAD( "maz-01.3b", 0x080000, 0x080000, CRC(4934fe21) SHA1(b852249f59906d69d32160ebaf9b4781193227e4) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "ke01.14f", 0x00000, 0x20000, CRC(8a106263) SHA1(229ab17403c2b8f4e89a90a8cda2f3c3a4b55d9e) )

	ROM_REGION( 0x100000, "oki2", 0 ) // banked
	ROM_LOAD( "maz-03.13f", 0x00000, 0x100000, CRC(a313c964) SHA1(4a3664c4e2c44a017a0ab6a6d4361799cbda57b5) )
ROM_END

ROM_START( pktgaldxa )
	ROM_REGION( 0x80000, "maincpu", 0 ) // DE102 code (encrypted)
	ROM_LOAD16_WORD_SWAP( "rom.12a",   0x00000, 0x80000, CRC(c4c7cc68) SHA1(d3841459096a37ffd96fbf9017d3176893034c01) ) // Version 3.00 Asia - Need to verify label

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "maz-02.2h", 0x00000, 0x100000, CRC(c9d35a59) SHA1(07b44c7d7d76b668b4d6ca5672bd1c2910228e68) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "maz-00.1b", 0x000000, 0x080000, CRC(fa3071f4) SHA1(72e7d920e9ca94f8cb166007a9e9e5426a201af8) )
	ROM_LOAD( "maz-01.3b", 0x080000, 0x080000, CRC(4934fe21) SHA1(b852249f59906d69d32160ebaf9b4781193227e4) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "ke01.14f", 0x00000, 0x20000, CRC(8a106263) SHA1(229ab17403c2b8f4e89a90a8cda2f3c3a4b55d9e) )

	ROM_REGION( 0x100000, "oki2", 0 ) // banked
	ROM_LOAD( "maz-03.13f", 0x00000, 0x100000, CRC(a313c964) SHA1(4a3664c4e2c44a017a0ab6a6d4361799cbda57b5) )
ROM_END

ROM_START( pktgaldxb )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE ("4.bin", 0x00000, 0x80000, CRC(67ce30aa) SHA1(c5228ed19eebbfb6d5f7cbfcb99734d9fcd5aba3) )
	ROM_LOAD16_BYTE( "5.bin", 0x00001, 0x80000, CRC(64cb4c33) SHA1(02f988f558113dd9a77079dee59e23583394fa98) )

	ROM_REGION( 0x400000, "tiles", 0 )
	ROM_LOAD16_BYTE( "11.bin", 0x000000, 0x80000, CRC(a8c8f1fd) SHA1(9fd5fa500967a1bd692abdbeef89ce195c8aecd4) )
	ROM_LOAD16_BYTE( "6.bin",  0x000001, 0x80000, CRC(0e3335a1) SHA1(2d6899336302d222e8404dde159e64911a8f94e6) )
	ROM_LOAD16_BYTE( "10.bin", 0x100000, 0x80000, CRC(9dd743a9) SHA1(dbc3e2bd044dbf21b04c174bd860969ee53b4050) )
	ROM_LOAD16_BYTE( "7.bin",  0x100001, 0x80000, CRC(0ebf12b5) SHA1(17b6c2ce21de3671d75d89a41317efddf5b49339) )
	ROM_LOAD16_BYTE( "9.bin",  0x200001, 0x80000, CRC(078f371c) SHA1(5b510a0f7f50c55cce1ffcc8f2e9c3432b23e352) )
	ROM_LOAD16_BYTE( "8.bin",  0x200000, 0x80000, CRC(40f5a032) SHA1(c2ad585ddbc3ef40c6214cb30b4d78a2cd0a9446) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x20000, CRC(8a106263) SHA1(229ab17403c2b8f4e89a90a8cda2f3c3a4b55d9e) )

	ROM_REGION( 0x100000, "oki2", 0 ) // banked
	ROM_LOAD( "3.bin", 0x00000, 0x80000, CRC(4638747b) SHA1(56d79cd8d4d7b41b71f1e942b5a5bf1bafc5c6e7) )
	ROM_LOAD( "2.bin", 0x80000, 0x80000, CRC(f841d995) SHA1(0ef2f8fd9be62b979862c3688e7aad34c7b0404d) )
ROM_END



void pktgaldx_state::driver_init()
{
	deco56_decrypt_gfx(machine(), "tiles");
	deco102_decrypt_cpu((uint16_t *)memregion("maincpu")->base(), m_decrypted_opcodes, 0x80000, 0x42ba, 0x00, 0x00);
}

} // anonymous namespace


GAME( 1992, pktgaldx,  0,        pktgaldx,  pktgaldx, pktgaldx_state,  driver_init, ROT0, "Data East Corporation",                        "Pocket Gal Deluxe (Europe v3.00)",          MACHINE_SUPPORTS_SAVE )
GAME( 1993, pktgaldxj, pktgaldx, pktgaldx,  pktgaldx, pktgaldx_state,  driver_init, ROT0, "Data East Corporation (Nihon System license)", "Pocket Gal Deluxe (Japan v3.00)",           MACHINE_SUPPORTS_SAVE )
GAME( 1992, pktgaldxa, pktgaldx, pktgaldx,  pktgaldx, pktgaldx_state,  driver_init, ROT0, "Data East Corporation",                        "Pocket Gal Deluxe (Asia v3.00)",            MACHINE_SUPPORTS_SAVE )
GAME( 1993, pktgaldxb, pktgaldx, pktgaldxb, pktgaldx, pktgaldxb_state, empty_init,  ROT0, "bootleg (Data West)",                          "Pocket Gal Deluxe (Europe v3.00, bootleg)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
