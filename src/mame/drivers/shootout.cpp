// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Phil Stroffolino, Bryan McPhail
/*******************************************************************************

    Shoot Out (USA)             (c) 1985 Data East USA      DE-0219
    Shoot Out (Japan)           (c) 1985 Data East USA      DE-0203
    Shoot Out (Korean bootleg)  (c) 1985 Data East USA      DE-0203 bootleg

    Shoot Out (Japan) is an interesting board, it runs on an earlier PCB design
    than the USA version, has no sound CPU, uses half as many sprites and
    unusually for a Deco Japanese game it is credited to 'Data East USA'.
    Perhaps the USA arm of Deco designed this game rather than the Japanese
    arm?

    Shoot Out (Korean bootleg) is based on the earlier DE-0203 board but
    strangely features the same encryption as used on the DE-0219 board. It
    also has some edited graphics.

    The DE-0219 PCB seems to have only one 12 MHz XTAL, some images with recognizable XTAL value can be found here:
    - http://www.jammarcade.net/images/2016/04/Shootout.jpg
    - http://thumbs.picclick.com/00/s/MTIwMFgxNjAw/z/7iIAAOSw5ClXxbrB/$/Data-East-Shootout-Arcade-Video-Game-Pcb-Circuit-_57.jpg


    Driver by:
        Ernesto Corvi (ernesto@imagina.com)
        Phil Stroffolino
        Shoot Out (Japan) and fixes added by Bryan McPhail (mish@tendril.co.uk)

    TODO:

    - Lots of unmapped memory reads and writes (sprram or vram mirrors, perhaps...), bg_vram is also read.
    - Does the korean bootleg really have the DECO 222 CPU? I think it should use the shootclr.003 prom to decrypt the opcodes.
      Something like -> rom [addr] = (rom [addr] & 0x0F) | proms [rom [addr] >> 4]]);

*******************************************************************************/

/*

    2003-06-01  Added cocktail support to shootout
    2003-10-08  Added cocktail support to shootouj/shootoub
    2003-10-21  Removed input port hack

*/

#include "emu.h"
#include "includes/shootout.h"

#include "cpu/m6502/m6502.h"
#include "sound/2203intf.h"
#include "machine/deco222.h"
#include "screen.h"
#include "speaker.h"

/*******************************************************************************/

WRITE8_MEMBER(shootout_state::bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x0f);
}

READ8_MEMBER(shootout_state::sound_cpu_command_r)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return m_soundlatch->read();
}

WRITE8_MEMBER(shootout_state::sound_cpu_command_w)
{
	m_soundlatch->write(data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	// Allow the other CPU to reply. This fixes the missing music on the title screen (parent set).
	m_maincpu->spin_until_time(attotime::from_usec(200));
}

WRITE8_MEMBER(shootout_state::flipscreen_w)
{
	flip_screen_set(data & 0x01);
}

/*
    This is actually a double BCD counter (2 BCD digits packet in a byte)...
    The first write is always 0x40, then when a coin is inserted it starts to count from 0x01 up to 0x99.
    When it reaches 99 credits, 0x99 is always written...

    On the shootoutj and shootoutb sets, it works as above but it counts up to 0x09 instead of 0x99 (Single BCD digit).

    Note:
    This should be an input for a BCD to 7-segment decoder (e.g. a 74LS47), but all the PCBs I've seen don't have 'onboard'
    display(s), so this was implemented as normal "coin counter" (after all, they both have the same goal: count credits ;))
*/
WRITE8_MEMBER(shootout_state::coincounter_w)
{
	if (data != m_ccnt_old_val)
	{
		// Do a coin pulse
		machine().bookkeeping().coin_counter_w(0, 0);
		machine().bookkeeping().coin_counter_w(0, 1);

		m_ccnt_old_val = data;
	}
}

/*******************************************************************************/

void shootout_state::shootout_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1000).portr("DSW1").w(FUNC(shootout_state::bankswitch_w));
	map(0x1001, 0x1001).portr("P1").w(FUNC(shootout_state::flipscreen_w));
	map(0x1002, 0x1002).portr("P2").w(FUNC(shootout_state::coincounter_w));
	map(0x1003, 0x1003).portr("DSW2").w(FUNC(shootout_state::sound_cpu_command_w));
	map(0x1004, 0x17ff).ram();
	map(0x1800, 0x19ff).ram().share("spriteram");
	map(0x2000, 0x27ff).ram().w(FUNC(shootout_state::textram_w)).share("textram");
	map(0x2800, 0x2fff).ram().w(FUNC(shootout_state::videoram_w)).share("videoram");
	map(0x4000, 0x7fff).bankr("bank1");
	map(0x8000, 0xffff).rom().region("maincpu", 0x0000);
}

void shootout_state::shootouj_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1000).portr("DSW1");
	map(0x1001, 0x1001).portr("P1");
	map(0x1002, 0x1002).portr("P2");
	map(0x1003, 0x1003).portr("DSW2");
	map(0x1004, 0x17ff).ram();
	map(0x1800, 0x1800).w(FUNC(shootout_state::coincounter_w));
	map(0x2000, 0x21ff).ram().share("spriteram");
	map(0x2800, 0x2801).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x3000, 0x37ff).ram().w(FUNC(shootout_state::textram_w)).share("textram");
	map(0x3800, 0x3fff).ram().w(FUNC(shootout_state::videoram_w)).share("videoram");
	map(0x4000, 0x7fff).bankr("bank1");
	map(0x8000, 0xffff).rom().region("maincpu", 0x0000);
}

/*******************************************************************************/

/* same as Tryout */
void shootout_state::shootout_sound_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x4000, 0x4001).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xa000, 0xa000).r(FUNC(shootout_state::sound_cpu_command_r));
	map(0xc000, 0xffff).rom().region("audiocpu", 0x0000);
	map(0xd000, 0xd000).nopw(); // Unknown, NOT irq/nmi mask (Always 0x80 ???)
}

/*******************************************************************************/

INPUT_CHANGED_MEMBER(shootout_state::coin_inserted)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( shootout )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, shootout_state,coin_inserted, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, shootout_state,coin_inserted, 0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )       PORT_DIPLOCATION( "DSW1:1,2" )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION( "DSW1:3,4" )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "DSW1:5" )  /* Manual lists this dip as "Unused" */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION( "DSW1:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION( "DSW1:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )            PORT_DIPLOCATION( "DSW1:8" )    /* Manual lists this dip as "Unused" */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION( "DSW2:1,2" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION( "DSW2:3,4" )
	PORT_DIPSETTING(    0x0c, "20,000 Every 70,000" )
	PORT_DIPSETTING(    0x08, "30,000 Every 80,000" )
	PORT_DIPSETTING(    0x04, "40,000 Every 90,000" )
	PORT_DIPSETTING(    0x00, "70,000" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION( "DSW2:5,6" )
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) // This needs to be low to allows both coins to be accepted...
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static INPUT_PORTS_START( shootouj )
	PORT_INCLUDE(shootout)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x20, "Company Copyright" )     PORT_DIPLOCATION( "DSW1:6" )
	PORT_DIPSETTING(    0x20, "Data East Corp" )
	PORT_DIPSETTING(    0x00, "Data East USA Inc" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION( "DSW2:3,4" )
	PORT_DIPSETTING(    0x0c, "20,000 Every 50,000" )
	PORT_DIPSETTING(    0x08, "30,000 Every 60,000" )
	PORT_DIPSETTING(    0x04, "50,000 Every 70,000" )
	PORT_DIPSETTING(    0x00, "70,000" )
INPUT_PORTS_END


static const gfx_layout char_layout =
{
	8,8,    /* 8*8 characters */
	0x400,  /* 1024 characters */
	2,  /* 2 bits per pixel */
	{ 0,4 },    /* the bitplanes are packed in the same byte */
	{ (0x2000*8)+0, (0x2000*8)+1, (0x2000*8)+2, (0x2000*8)+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};
static const gfx_layout sprite_layout =
{
	16,16,  /* 16*16 sprites */
	0x800,  /* 2048 sprites */
	3,  /* 3 bits per pixel */
	{ 0*0x10000*8, 1*0x10000*8, 2*0x10000*8 },  /* the bitplanes are separated */
	{ 128+0, 128+1, 128+2, 128+3, 128+4, 128+5, 128+6, 128+7, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every char takes 32 consecutive bytes */
};
static const gfx_layout tile_layout =
{
	8,8,    /* 8*8 characters */
	0x800,  /* 2048 characters */
	2,  /* 2 bits per pixel */
	{ 0,4 },    /* the bitplanes are packed in the same byte */
	{ (0x4000*8)+0, (0x4000*8)+1, (0x4000*8)+2, (0x4000*8)+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( gfx_shootout )
	GFXDECODE_ENTRY( "gfx1", 0, char_layout,   16*4+8*8, 16 ) /* characters */
	GFXDECODE_ENTRY( "gfx2", 0, sprite_layout, 16*4,     8  ) /* sprites */
	GFXDECODE_ENTRY( "gfx3", 0, tile_layout,   0,        16 ) /* tiles */
GFXDECODE_END

void shootout_state::machine_reset ()
{
	m_ccnt_old_val = 0x40;
}

void shootout_state::shootout(machine_config &config)
{
	/* basic machine hardware */
	DECO_222(config, m_maincpu, XTAL(12'000'000) / 6); // 2 MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &shootout_state::shootout_map);

	M6502(config, m_audiocpu, XTAL(12'000'000) / 8); // 1.5 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &shootout_state::shootout_sound_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// Guessed parameters based on the 12 MHz XTAL, but they seem reasonable (TODO: Real PCB measurements)
	screen.set_raw(XTAL(12'000'000) / 2, 384, 0, 256, 262, 8, 248);
	screen.set_screen_update(FUNC(shootout_state::screen_update_shootout));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_shootout);
	PALETTE(config, m_palette, FUNC(shootout_state::shootout_palette), 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2203_device &ymsnd(YM2203(config, "ymsnd", XTAL(12'000'000) / 8)); // 1.5 MHz
	ymsnd.irq_handler().set_inputline(m_audiocpu, M6502_IRQ_LINE);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.00);
}


void shootout_state::shootouj(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, XTAL(12'000'000) / 6); // 2 MHz? (Assuming the same XTAL as DE-0219 pcb)
	m_maincpu->set_addrmap(AS_PROGRAM, &shootout_state::shootouj_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// Guessed parameters based on the 12 MHz XTAL, but they seem reasonable (TODO: Real PCB measurements)
	screen.set_raw (XTAL(12'000'000) / 2, 384, 0, 256, 262, 8, 248);
	screen.set_screen_update(FUNC(shootout_state::screen_update_shootouj));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_shootout);
	PALETTE(config, m_palette, FUNC(shootout_state::shootout_palette), 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2203_device &ymsnd(YM2203(config, "ymsnd", XTAL(12'000'000) / 8)); // 1.5 MHz (Assuming the same XTAL as DE-0219 pcb)
	ymsnd.irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);
	ymsnd.port_a_write_callback().set(FUNC(shootout_state::bankswitch_w));
	ymsnd.port_b_write_callback().set(FUNC(shootout_state::flipscreen_w));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.00);
}

void shootout_state::shootouk(machine_config &config)
{
	shootouj(config);
	/* the Korean 'bootleg' has the usual DECO222 style encryption */
	DECO_222(config.replace(), m_maincpu, XTAL(12'000'000) / 6); // 2 MHz? (Assuming the same XTAL as DE-0219 pcb)
	m_maincpu->set_addrmap(AS_PROGRAM, &shootout_state::shootouj_map);
}



ROM_START( shootout )
	ROM_REGION( 2 * 0x14000, "maincpu", 0 ) // 80k for code + 80k for decrypted opcodes
	ROM_LOAD( "cu00.b1",        0x00000, 0x8000, CRC(090edeb6) SHA1(ab849d123dacf3947b1ebd29b70a20e066911a60) ) /* opcodes encrypted */
	ROM_LOAD( "cu02.c3",        0x08000, 0x8000, CRC(2a913730) SHA1(584488278d58c4d34a2eebeaf39518f87cf5eecd) ) /* opcodes encrypted */
	ROM_LOAD( "cu01.c1",        0x10000, 0x4000, CRC(8843c3ae) SHA1(c58ed4acac566f890cadf62bcbcced07a59243fc) ) /* opcodes encrypted */

	ROM_REGION( 0x4000, "audiocpu", 0 )
	ROM_LOAD( "cu09.j1",        0x00000, 0x4000, CRC(c4cbd558) SHA1(0e940ae99febc1161e5f35550aa75afca88cb5e9) ) /* Sound CPU */

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "cu11.h19",       0x00000, 0x4000, CRC(eff00460) SHA1(15daaa3d3125a981a26f31d43283faa5be26e96b) ) /* foreground characters */

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "cu04.c7",        0x00000, 0x8000, CRC(ceea6b20) SHA1(9fe363668db2e2759b3c531b4d7f23c65f2e8035) )   /* sprites */
	ROM_LOAD( "cu03.c5",        0x08000, 0x8000, CRC(b786bb3e) SHA1(5a209f01914ca4b206138d738a34640e0bcb3185) )
	ROM_LOAD( "cu06.c10",       0x10000, 0x8000, CRC(2ec1d17f) SHA1(74f0579a5ab3daf5d1290d3c15459f0f9b67bf79) )
	ROM_LOAD( "cu05.c9",        0x18000, 0x8000, CRC(dd038b85) SHA1(b1c3c1ab17c36a1c77726b5e485fc01581a4d97d) )
	ROM_LOAD( "cu08.c13",       0x20000, 0x8000, CRC(91290933) SHA1(60487f4eaf2e6c50b24c0f8fbd7abf92c04a342a) )
	ROM_LOAD( "cu07.c12",       0x28000, 0x8000, CRC(19b6b94f) SHA1(292264811206916af41d133f81dfd93c44f59a96) )

	ROM_REGION( 0x08000, "gfx3", 0 )
	ROM_LOAD( "cu10.h17",       0x00000, 0x2000, CRC(3854c877) SHA1(2c8fe4591553ce798c907849e3dbd410e4fe424c) ) /* background tiles */
	ROM_CONTINUE(               0x04000, 0x2000 )
	ROM_CONTINUE(               0x02000, 0x2000 )
	ROM_CONTINUE(               0x06000, 0x2000 )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "gb08.k10",       0x0000, 0x0100, CRC(509c65b6) SHA1(4cec37065a799ced4e7b6552f267aacc7f54ffe3) )
	ROM_LOAD( "gb09.k6",        0x0100, 0x0100, CRC(aa090565) SHA1(e289e77ec3402e86d93b873c0fa064f3e6277a62) )  /* priority encoder? (not used) */
ROM_END

ROM_START( shootoutj )
	ROM_REGION( 0x14000, "maincpu", 0 ) // 80k for code
	ROM_LOAD( "cg02.bin",    0x00000, 0x8000, CRC(8fc5d632) SHA1(809ac4eba09972229fe741c96fa8036d7139b6a8) )
	ROM_LOAD( "cg00.bin",    0x08000, 0x8000, CRC(ef6ced1e) SHA1(feea508c7a60fc6cde1efee52cba628accd26028) )
	ROM_LOAD( "cg01.bin",    0x10000, 0x4000, CRC(74cf11ca) SHA1(59edbc4633cd560e7b928b33e4c42d0125332a1b) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "cu11.h19",       0x00000, 0x4000, CRC(eff00460) SHA1(15daaa3d3125a981a26f31d43283faa5be26e96b) ) /* foreground characters */

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "cg03.bin",    0x00000, 0x8000, CRC(5252ec19) SHA1(c6848a815badd8845f91e898b0a52b7f12ed8a39) )  /* sprites */
	ROM_LOAD( "cg04.bin",    0x10000, 0x8000, CRC(db06cfe9) SHA1(e13c16232f54fe8467c21e0218c87606a19dd25c) )
	ROM_LOAD( "cg05.bin",    0x20000, 0x8000, CRC(d634d6b8) SHA1(e2ddd12b1b3fb0063104d414f0574b94dbfa0403) )

	ROM_REGION( 0x08000, "gfx3", 0 )
	ROM_LOAD( "cu10.h17",       0x00000, 0x2000, CRC(3854c877) SHA1(2c8fe4591553ce798c907849e3dbd410e4fe424c) ) /* background tiles */
	ROM_CONTINUE(               0x04000, 0x2000 )
	ROM_CONTINUE(               0x02000, 0x2000 )
	ROM_CONTINUE(               0x06000, 0x2000 )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "gb08.k10",       0x0000, 0x0100, CRC(509c65b6) SHA1(4cec37065a799ced4e7b6552f267aacc7f54ffe3) )
	ROM_LOAD( "gb09.k6",        0x0100, 0x0100, CRC(aa090565) SHA1(e289e77ec3402e86d93b873c0fa064f3e6277a62) )  /* priority encoder? (not used) */
ROM_END

ROM_START( shootoutb )
	ROM_REGION( 2 * 0x14000, "maincpu", 0 ) // 80k for code + 80k for decrypted opcodes
	ROM_LOAD( "shootout.006", 0x00000, 0x8000, CRC(2c054888) SHA1(cb0de2f7d743506789626304e6bcbbc292fbe8bc) )
	ROM_LOAD( "shootout.008", 0x08000, 0x8000, CRC(9651b656) SHA1(e90eddf2833ef36fa73b7b8d81d28443d2f60220) )
	ROM_LOAD( "cg01.bin",     0x10000, 0x4000, CRC(74cf11ca) SHA1(59edbc4633cd560e7b928b33e4c42d0125332a1b) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "cu11.h19",       0x00000, 0x4000, CRC(eff00460) SHA1(15daaa3d3125a981a26f31d43283faa5be26e96b) ) /* foreground characters */

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "shootout.005",   0x00000, 0x8000, CRC(e6357ba3) SHA1(1ceb46450a0c4f6f7f7109601ad6617f08364df5) )   /* sprites */
	ROM_LOAD( "shootout.004",   0x10000, 0x8000, CRC(7f422c93) SHA1(97d9a17956e838801c416461b020876c780bf260) )
	ROM_LOAD( "shootout.003",   0x20000, 0x8000, CRC(eea94535) SHA1(65819b7925ecd9ae6e62decb3b0164f627b73fe5) )

	ROM_REGION( 0x08000, "gfx3", 0 )
	ROM_LOAD( "cu10.h17",       0x00000, 0x2000, CRC(3854c877) SHA1(2c8fe4591553ce798c907849e3dbd410e4fe424c) ) /* background tiles */
	ROM_CONTINUE(               0x04000, 0x2000 )
	ROM_CONTINUE(               0x02000, 0x2000 )
	ROM_CONTINUE(               0x06000, 0x2000 )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "gb08.k10",       0x0000, 0x0100, CRC(509c65b6) SHA1(4cec37065a799ced4e7b6552f267aacc7f54ffe3) )
	ROM_LOAD( "gb09.k6",        0x0100, 0x0100, CRC(aa090565) SHA1(e289e77ec3402e86d93b873c0fa064f3e6277a62) )  /* priority encoder? (not used) */
	ROM_LOAD( "shootclr.003",   0x0200, 0x0020, CRC(6b0c2942) SHA1(7d25acc753923b265792fc78f8fc70175c0e0ec2) )  /* opcode decrypt table (bootleg only) */
ROM_END


void shootout_state::init_shootout()
{
	membank("bank1")->configure_entries(0, 16, memregion("maincpu")->base() + 0x8000, 0x4000);
}


GAME( 1985, shootout,  0,        shootout, shootout, shootout_state, init_shootout, ROT0, "Data East USA",         "Shoot Out (US)",             MACHINE_SUPPORTS_SAVE )
GAME( 1985, shootoutj, shootout, shootouj, shootouj, shootout_state, init_shootout, ROT0, "Data East Corporation", "Shoot Out (Japan)",          MACHINE_SUPPORTS_SAVE )
GAME( 1985, shootoutb, shootout, shootouk, shootout, shootout_state, init_shootout, ROT0, "bootleg",               "Shoot Out (Korean Bootleg)", MACHINE_SUPPORTS_SAVE )
