// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************
  Munch Mobile
  (C) 1983 SNK

  2 Z80s
  2 AY-8910s
  15 MHz crystal

  Known Issues:
    - it's unclear if mirroring the videoram chunks is correct behavior
    - several unmapped registers

Stephh's notes (based on the game Z80 code and some tests) :

  - The "Continue after Game Over" Dip Switch (DSW1:1) allows the player
    to continue from where he lost his last life when he starts a new game.
    IMO, this is a debug feature (as often with SNK games) as there is
    NO continue routine nor text for it in the ROMS.
    See code at 0x013a ('joyfulr') or 0x013e ('mnchmobl') for more infos.
  - There is extra code at 0x1de2 in 'mnchmobl' but it doesn't seem to be used.

- DIPs are now verified from Munch Mobile manual and playtesting.

***************************************************************************/

#include "emu.h"
#include "munchmo.h"

#include "cpu/z80/z80.h"
#include "screen.h"
#include "speaker.h"


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE_LINE_MEMBER(munchmo_state::nmi_enable_w)
{
	m_nmi_enable = state;
}

/* trusted thru schematics, NMI and IRQ triggers at vblank, at the same time (!) */
WRITE_LINE_MEMBER(munchmo_state::vblank_irq)
{
	if (state)
	{
		if (m_nmi_enable)
			m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

		m_maincpu->set_input_line(0, ASSERT_LINE);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
}

IRQ_CALLBACK_MEMBER(munchmo_state::generic_irq_ack)
{
	device.execute().set_input_line(0, CLEAR_LINE);
	return 0xff;
}

void munchmo_state::nmi_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void munchmo_state::sound_nmi_ack_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

uint8_t munchmo_state::ay1reset_r()
{
	m_ay8910[0]->reset_w();
	return 0;
}

uint8_t munchmo_state::ay2reset_r()
{
	m_ay8910[1]->reset_w();
	return 0;
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void munchmo_state::mnchmobl_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x83ff).ram();
	map(0xa000, 0xa3ff).mirror(0x0400).ram().share(m_sprite_xpos);
	map(0xa800, 0xabff).mirror(0x0400).ram().share(m_sprite_tile);
	map(0xb000, 0xb3ff).mirror(0x0400).ram().share(m_sprite_attr);
	map(0xb800, 0xb8ff).mirror(0x0100).ram().share(m_videoram);
	map(0xbaba, 0xbaba).nopw(); /* ? */
	map(0xbc00, 0xbc7f).ram().share(m_status_vram);
	map(0xbe00, 0xbe00).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xbe01, 0xbe01).select(0x0070).lw8(NAME([this] (offs_t offset, u8 data){ m_mainlatch->write_d0(offset >> 4, data); }));
	map(0xbe02, 0xbe02).portr("DSW1");
	map(0xbe03, 0xbe03).portr("DSW2");
	map(0xbf00, 0xbf00).w(FUNC(munchmo_state::nmi_ack_w)); // CNI 1-8C
	map(0xbf01, 0xbf01).portr("SYSTEM");
	map(0xbf02, 0xbf02).portr("P1");
	map(0xbf03, 0xbf03).portr("P2");
	map(0xbf04, 0xbf07).writeonly().share(m_vreg); // MY0 1-8C
}

/* memory map provided thru schematics */
void munchmo_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x4000, 0x4fff).w(m_ay8910[0], FUNC(ay8910_device::data_w));
	map(0x5000, 0x5fff).w(m_ay8910[0], FUNC(ay8910_device::address_w));
	map(0x6000, 0x6fff).w(m_ay8910[1], FUNC(ay8910_device::data_w));
	map(0x7000, 0x7fff).w(m_ay8910[1], FUNC(ay8910_device::address_w));
	map(0x8000, 0x9fff).r(FUNC(munchmo_state::ay1reset_r)).w(m_ay8910[0], FUNC(ay8910_device::reset_w));
	map(0xa000, 0xbfff).r(FUNC(munchmo_state::ay2reset_r)).w(m_ay8910[1], FUNC(ay8910_device::reset_w));
	map(0xc000, 0xdfff).w(FUNC(munchmo_state::sound_nmi_ack_w)); // NCL 1-8H
	map(0xe000, 0xe7ff).mirror(0x1800).ram(); // is mirror ok?
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mnchmobl )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )     PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )   PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )   PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )  PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_2WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )     PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )   PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )   PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
		/* See notes about this DIP */
	PORT_DIPNAME( 0x01, 0x00, "Continue after Game Over (Cheat)" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )

	PORT_DIPNAME( 0x1e, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:2,3,4,5")
	PORT_DIPSETTING(    0x14, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x16, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x12, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_8C ) )

// Duplicate Settings
// PORT_DIPSETTING(    0x1e, DEF_STR( 1C_1C ) )
// PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
// PORT_DIPSETTING(    0x1a, DEF_STR( 1C_1C ) )
// PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )

	PORT_DIPNAME( 0xe0, 0x00, "1st Bonus" ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x40, "30000" )
	PORT_DIPSETTING(    0x60, "40000" )
	PORT_DIPSETTING(    0x80, "50000" )
	PORT_DIPSETTING(    0xa0, "60000" )
	PORT_DIPSETTING(    0xc0, "70000" )
	PORT_DIPSETTING(    0xe0, "No Bonus" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, "2nd Bonus (1st+)" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x01, "40000" )
	PORT_DIPSETTING(    0x02, "100000" )
	PORT_DIPSETTING(    0x03, "No Bonus" )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x10, 0x00, "Freeze" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, "Additional Bonus (2nd Bonus Value)" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout char_layout =
{
	8,8,
	256,
	4,
	{ 0, 8, 256*128,256*128+8 },
	{ 7,6,5,4,3,2,1,0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128
};

static const gfx_layout tile_layout =
{
	8,8,
	0x100,
	4,
	{ 8,12,0,4 },
	{ 0,0,1,1,2,2,3,3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128
};

static const gfx_layout sprite_layout1 =
{
	32,32,
	128,
	3,
	{ 0x4000*8,0x2000*8,0 },
	{
		7,7,6,6,5,5,4,4,3,3,2,2,1,1,0,0,
		0x8000+7,0x8000+7,0x8000+6,0x8000+6,0x8000+5,0x8000+5,0x8000+4,0x8000+4,
		0x8000+3,0x8000+3,0x8000+2,0x8000+2,0x8000+1,0x8000+1,0x8000+0,0x8000+0
	},
	{
			0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8,10*8,11*8,12*8,13*8,14*8,15*8,
		16*8,17*8,18*8,19*8,20*8,21*8,22*8,23*8,
		24*8,25*8,26*8,27*8,28*8,29*8,30*8,31*8
	},
	256
};

static const gfx_layout sprite_layout2 =
{
	32,32,
	128,
	3,
	{ 0,0,0 },
	{
		7,7,6,6,5,5,4,4,3,3,2,2,1,1,0,0,
		0x8000+7,0x8000+7,0x8000+6,0x8000+6,0x8000+5,0x8000+5,0x8000+4,0x8000+4,
		0x8000+3,0x8000+3,0x8000+2,0x8000+2,0x8000+1,0x8000+1,0x8000+0,0x8000+0
	},
	{
			0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8,10*8,11*8,12*8,13*8,14*8,15*8,
		16*8,17*8,18*8,19*8,20*8,21*8,22*8,23*8,
		24*8,25*8,26*8,27*8,28*8,29*8,30*8,31*8
	},
	256
};

static GFXDECODE_START( gfx_mnchmobl )
	GFXDECODE_ENTRY( "gfx1", 0,      char_layout,      0,  4 )  /* colors   0- 63 */
	GFXDECODE_ENTRY( "gfx2", 0x1000, tile_layout,     64,  4 )  /* colors  64-127 */
	GFXDECODE_ENTRY( "gfx3", 0,      sprite_layout1, 128, 16 )  /* colors 128-255 */
	GFXDECODE_ENTRY( "gfx4", 0,      sprite_layout2, 128, 16 )  /* colors 128-255 */
GFXDECODE_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void munchmo_state::machine_start()
{
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_nmi_enable));
}

void munchmo_state::mnchmobl(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(15'000'000)/4); // from pin 13 of XTAL-driven 163
	m_maincpu->set_addrmap(AS_PROGRAM, &munchmo_state::mnchmobl_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(munchmo_state::generic_irq_ack)); // IORQ clears flip-flop at 1-2C

	Z80(config, m_audiocpu, XTAL(15'000'000)/8); // from pin 12 of XTAL-driven 163
	m_audiocpu->set_addrmap(AS_PROGRAM, &munchmo_state::sound_map);
	m_audiocpu->set_irq_acknowledge_callback(FUNC(munchmo_state::generic_irq_ack)); // IORQ clears flip-flop at 1-7H

	LS259(config, m_mainlatch, 0); // 12E
	m_mainlatch->q_out_cb<0>().set(FUNC(munchmo_state::palette_bank_0_w)); // BCL0 2-11E
	m_mainlatch->q_out_cb<1>().set(FUNC(munchmo_state::palette_bank_1_w)); // BCL1 2-11E
	m_mainlatch->q_out_cb<2>().set_nop(); // CL2 2-11E
	m_mainlatch->q_out_cb<3>().set_nop(); // CL3 2-11E
	m_mainlatch->q_out_cb<4>().set(FUNC(munchmo_state::flipscreen_w)); // INV
	m_mainlatch->q_out_cb<5>().set_nop(); // DISP
	m_mainlatch->q_out_cb<6>().set(FUNC(munchmo_state::nmi_enable_w)); // ENI 1-10C

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(256+32+32, 256);
	screen.set_visarea(0, 255+32+32,0, 255-16);
	screen.set_screen_update(FUNC(munchmo_state::screen_update));
	screen.screen_vblank().set(FUNC(munchmo_state::vblank_irq));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mnchmobl);
	PALETTE(config, m_palette, FUNC(munchmo_state::munchmo_palette), 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch).data_pending_callback().set_inputline(m_audiocpu, 0, ASSERT_LINE);

	/* AY clock speeds confirmed to match known recording */
	AY8910(config, m_ay8910[0], XTAL(15'000'000)/8);
	//m_ay8910[0]->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay8910[0]->add_route(ALL_OUTPUTS, "mono", 0.50);

	AY8910(config, m_ay8910[1], XTAL(15'000'000)/8);
	//m_ay8910[1]->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay8910[1]->add_route(ALL_OUTPUTS, "mono", 0.50);
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( joyfulr )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for CPUA */
	ROM_LOAD( "m1j.10e", 0x0000, 0x2000, CRC(1fe86e25) SHA1(e13abc20741dfd8a260f354efda3b3a25c820343) )
	ROM_LOAD( "m2j.10d", 0x2000, 0x2000, CRC(b144b9a6) SHA1(efed5fd6ba941b2baa7c8a17fe7323172c8fb17c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mu.2j",   0x0000, 0x2000, CRC(420adbd4) SHA1(3da18cda97ca604dc074b50c4f36287e0679224a) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "s1.10a",  0x0000, 0x1000, CRC(c0bcc301) SHA1(b8961e7bbced4dfe9c72f839ea9b89d3f2e629b2) ) /* characters */
	ROM_LOAD( "s2.10b",  0x1000, 0x1000, CRC(96aa11ca) SHA1(84438d6b27d520e95b8706c91c5c20de1785604c) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "b1.2c",   0x0000, 0x1000, CRC(8ce3a403) SHA1(eec5813076c31bb8534f7d1f83f2a397e552ed69) ) /* tile layout */
	ROM_LOAD( "b2.2b",   0x1000, 0x1000, CRC(0df28913) SHA1(485700d3b7f2bfcb970e8f9edb7d18ed9a708bd2) ) /* 4x8 tiles */

	ROM_REGION( 0x6000, "gfx3", 0 )
	ROM_LOAD( "f1j.1g",  0x0000, 0x2000, CRC(93c3c17e) SHA1(902f458c4efe74187a58a3c1ecd146e343657977) ) /* sprites */
	ROM_LOAD( "f2j.3g",  0x2000, 0x2000, CRC(b3fb5bd2) SHA1(51ff8b0bec092c9404944d6069c4493049604cb8) )
	ROM_LOAD( "f3j.5g",  0x4000, 0x2000, CRC(772a7527) SHA1(fe561d5323472e79051614a374e92aab17636055) )

	ROM_REGION( 0x2000, "gfx4", 0 )
	ROM_LOAD( "h",       0x0000, 0x2000, CRC(332584de) SHA1(9ef75a77e6cc298a315d80b7f2d24414827c7063) ) /* monochrome sprites */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "a2001.clr", 0x0000, 0x0100, CRC(1b16b907) SHA1(fc362174af128827b0b8119fdc1b5569598c087a) ) /* color prom */
ROM_END

ROM_START( mnchmobl )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for CPUA */
	ROM_LOAD( "m1.10e",  0x0000, 0x2000, CRC(a4bebc6a) SHA1(7c13b2b87168dee3c1b8e931487a56d0a528386e) )
	ROM_LOAD( "m2.10d",  0x2000, 0x2000, CRC(f502d466) SHA1(4da5a32b3903fb7fbef38fc385408b9390b5f57f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mu.2j",   0x0000, 0x2000, CRC(420adbd4) SHA1(3da18cda97ca604dc074b50c4f36287e0679224a) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "s1.10a",  0x0000, 0x1000, CRC(c0bcc301) SHA1(b8961e7bbced4dfe9c72f839ea9b89d3f2e629b2) ) /* characters */
	ROM_LOAD( "s2.10b",  0x1000, 0x1000, CRC(96aa11ca) SHA1(84438d6b27d520e95b8706c91c5c20de1785604c) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "b1.2c",   0x0000, 0x1000, CRC(8ce3a403) SHA1(eec5813076c31bb8534f7d1f83f2a397e552ed69) ) /* tile layout */
	ROM_LOAD( "b2.2b",   0x1000, 0x1000, CRC(0df28913) SHA1(485700d3b7f2bfcb970e8f9edb7d18ed9a708bd2) ) /* 4x8 tiles */

	ROM_REGION( 0x6000, "gfx3", 0 )
	ROM_LOAD( "f1.1g",   0x0000, 0x2000, CRC(b75411d4) SHA1(d058a6c219676f8ba4e498215f5716c630bb1d20) ) /* sprites */
	ROM_LOAD( "f2.3g",   0x2000, 0x2000, CRC(539a43ba) SHA1(a7b30c41d9fdb420ec8f0c6441432c1b2b69c4be) )
	ROM_LOAD( "f3.5g",   0x4000, 0x2000, CRC(ec996706) SHA1(e71e99061ce83068b0ec60ae97759a9d78c7cdf9) )

	ROM_REGION( 0x2000, "gfx4", 0 )
	ROM_LOAD( "h",       0x0000, 0x2000, CRC(332584de) SHA1(9ef75a77e6cc298a315d80b7f2d24414827c7063) ) /* monochrome sprites */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "a2001.clr", 0x0000, 0x0100, CRC(1b16b907) SHA1(fc362174af128827b0b8119fdc1b5569598c087a) ) /* color prom */
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1983, joyfulr,  0,        mnchmobl, mnchmobl, munchmo_state, empty_init, ROT270, "SNK",                   "Joyful Road (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1983, mnchmobl, joyfulr,  mnchmobl, mnchmobl, munchmo_state, empty_init, ROT270, "SNK (Centuri license)", "Munch Mobile (US)",   MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
