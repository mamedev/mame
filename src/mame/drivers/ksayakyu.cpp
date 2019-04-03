// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*
Kusayakyuu (Sandlot Baseball)
(c)1985 Taito

driver by Tomasz Slanina

TODO:
- sprite glitches (sometimes) .. missing vertical flip flag? <- (*)
- sound cpu int freq (timer ? $a010 writes ?)

(*) this was caused by flip Y being hooked up as bit 6 in text videoram attribute. -AS

M6100097A

---------------
CPU/Sound board
---------------
AGC-1 ECB-405010 ASY-410011

CPU  : Zilog Z8400APS(Z80A)
Sound: Zilog Z8400APS(Z80A) AY-3-8910(x2) DAC
OSC  : 18.432MHz

ROMs:
 1.3A [6607976d] - Main program (27128)
 2.3B [a289de5c] |
 3.3C [bb4104a5] /
 4.3D [db0ca023] - Main program (2764)

 5.5F [b0b21817] - Samples
 6.5H [17986662] |
 7.5J [ad242eda] /
 8.5L [3fbae535] - Samples + Sound program?

SRAM:
Hitachi HM6116 (x1)

-----------
Video Board
-----------
AGC-1 ECB-405011 ASY-410011

ROMs:
 9.3J [ef8411dd] - Graphics (2764)
10.3K [1bdee573] |
11.3L [c5859887] /

12.7N [ae8d6ed2] - Graphics (2764)
13.7R [0acb8c61] /

14.9J [982d06f0] - Graphics (2764)
15.9K [dc126df9] |
16.9M [574a172d] |
17.9N [a4c4e4ce] |
18.9R [9d75b104] /

PROM:
6309-1.9F

SRAM:
93422(256x4) (x4)

*/

#include "emu.h"
#include "includes/ksayakyu.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "screen.h"
#include "speaker.h"


#define MAIN_CLOCK XTAL(18'432'000)


WRITE8_MEMBER(ksayakyu_state::bank_select_w)
{
	/*
	    bits:
	    76543210
	           x - ROM bank
	    xxxxxxx  - unused ?

	*/
	membank("bank1")->set_entry(data & 0x01);
}

WRITE8_MEMBER(ksayakyu_state::latch_w)
{
	m_sound_status &= ~0x80;
	m_soundlatch->write(data | 0x80);
}

READ8_MEMBER(ksayakyu_state::sound_status_r)
{
	return m_sound_status | 4;
}

WRITE8_MEMBER(ksayakyu_state::tomaincpu_w)
{
	m_sound_status |= 0x80;
	m_soundlatch->write(data);
}

READ8_MEMBER(ksayakyu_state::int_ack_r)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0xff; // value not used
}

void ksayakyu_state::maincpu_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr("bank1");
	map(0x8000, 0x9fff).rom();
	map(0xa000, 0xa7ff).ram();
	map(0xa800, 0xa800).portr("P1");
	map(0xa801, 0xa801).portr("P2");
	map(0xa802, 0xa802).portr("DSW");
	map(0xa803, 0xa803).nopr(); /* watchdog ? */
	map(0xa804, 0xa804).w(FUNC(ksayakyu_state::ksayakyu_videoctrl_w));
	map(0xa805, 0xa805).w(FUNC(ksayakyu_state::latch_w));
	map(0xa806, 0xa806).r(FUNC(ksayakyu_state::sound_status_r));
	map(0xa807, 0xa807).r(FUNC(ksayakyu_state::int_ack_r));
	map(0xa808, 0xa808).w(FUNC(ksayakyu_state::bank_select_w));
	map(0xb000, 0xb7ff).ram().w(FUNC(ksayakyu_state::ksayakyu_videoram_w)).share("videoram");
	map(0xb800, 0xbfff).ram().share("spriteram");
}

void ksayakyu_state::soundcpu_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram();
	map(0xa001, 0xa001).r("ay1", FUNC(ay8910_device::data_r));
	map(0xa002, 0xa003).w("ay1", FUNC(ay8910_device::data_address_w));
	map(0xa006, 0xa007).w("ay2", FUNC(ay8910_device::data_address_w));
	map(0xa008, 0xa008).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xa00c, 0xa00c).w(FUNC(ksayakyu_state::tomaincpu_w));
	map(0xa010, 0xa010).nopw(); //a timer of some sort?
}

static INPUT_PORTS_START( ksayakyu )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x040, 0x00, "Continue" )
	PORT_DIPSETTING(    0x00, "7th inning" )
	PORT_DIPSETTING(    0x040, "1st inning" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END

WRITE8_MEMBER(ksayakyu_state::dummy1_w)
{
//  printf("%02x 1\n", data);
}

WRITE8_MEMBER(ksayakyu_state::dummy2_w)
{
//  printf("%02x 2\n", data);
}

WRITE8_MEMBER(ksayakyu_state::dummy3_w)
{
//  printf("%02x 3\n", data);
}

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};


static const gfx_layout charlayout2 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2)},
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	8*8*4
};

static GFXDECODE_START( gfx_ksayakyu )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, charlayout2,  0x80, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 16 )
GFXDECODE_END


void ksayakyu_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 2, &ROM[0x10000], 0x4000);

	save_item(NAME(m_sound_status));
	save_item(NAME(m_video_ctrl));
	save_item(NAME(m_flipscreen));
}

void ksayakyu_state::machine_reset()
{
	m_sound_status = 0xff;
	m_video_ctrl = 0;
	m_flipscreen = 0;
}

void ksayakyu_state::ksayakyu(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MAIN_CLOCK/8); //divider is guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &ksayakyu_state::maincpu_map);

	z80_device &audiocpu(Z80(config, "audiocpu", MAIN_CLOCK/8)); //divider is guessed, controls DAC tempo
	audiocpu.set_addrmap(AS_PROGRAM, &ksayakyu_state::soundcpu_map);
	audiocpu.set_periodic_int(FUNC(ksayakyu_state::irq0_line_hold), attotime::from_hz(60)); //guess, controls music tempo

	config.m_minimum_quantum = attotime::from_hz(60000);


	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(ksayakyu_state::screen_update_ksayakyu));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 0, ASSERT_LINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ksayakyu);
	PALETTE(config, m_palette, FUNC(ksayakyu_state::ksayakyu_palette), 256);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ay8910_device &ay1(AY8910(config, "ay1", MAIN_CLOCK/16)); //unknown clock
	ay1.port_a_read_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	ay1.port_b_write_callback().set(FUNC(ksayakyu_state::dummy1_w));
	ay1.add_route(ALL_OUTPUTS, "speaker", 0.25);

	ay8910_device &ay2(AY8910(config, "ay2", MAIN_CLOCK/16)); //unknown clock
	ay2.port_a_write_callback().set(FUNC(ksayakyu_state::dummy2_w));
	ay2.port_b_write_callback().set(FUNC(ksayakyu_state::dummy3_w));
	ay2.add_route(ALL_OUTPUTS, "speaker", 0.25);

	DAC_6BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}

ROM_START( ksayakyu )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1.3a", 0x00000, 0x4000, CRC(6607976d) SHA1(23b15cae04922e54faf35a5a499ba5064b18ed46) )
	ROM_LOAD( "2.3b", 0x04000, 0x4000, CRC(a289de5c) SHA1(d3b14364ef77ca74ac79c5099cf0e8f3baa97612) )
	ROM_RELOAD(       0x10000, 0x4000 )
	ROM_LOAD( "4.3d", 0x08000, 0x2000, CRC(db0ca023) SHA1(1356fa0239209dea6a4ac0af177fe8be47f12cd0) )
	ROM_LOAD( "3.3c", 0x14000, 0x4000, CRC(bb4104a5) SHA1(1f793c4431a3476eeb92556228bf855efb73fb83) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "8.5l",  0x0000, 0x2000, CRC(3fbae535) SHA1(87b66091e33eee66f4d63c213a5a19b70e7b1e5a) )
	ROM_LOAD( "7.5j",  0x2000, 0x2000, CRC(ad242eda) SHA1(928c04e4a48077ec9db148cb71d52ebfe01efa53) )
	ROM_LOAD( "6.5h",  0x4000, 0x2000, CRC(17986662) SHA1(81b65e381b923c5544f4708efef09f0894c716b2) )
	ROM_LOAD( "5.5f",  0x6000, 0x2000, CRC(b0b21817) SHA1(da2a1a6865dbc335775fa2e0ad0fb899be95af03) )

	ROM_REGION( 0x6000, "gfx1", 0  )
	ROM_LOAD( "9.3j",  0x0000, 0x2000, CRC(ef8411dd) SHA1(1dbc959d3aec9face19b2a5ae873ca34bfeff5cd) )
	ROM_LOAD( "10.3k", 0x2000, 0x2000, CRC(1bdee573) SHA1(7b92a8133cb83404505d21f462e3ca6c85647dca) )
	ROM_LOAD( "11.3l", 0x4000, 0x2000, CRC(c5859887) SHA1(41685fb8f8e7c44acd5e0e3ccc629e5f64a59fbd) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "14.9j", 0x0000, 0x2000, CRC(982d06f0) SHA1(e107c56ee4f2695a790b8cec6d52337ba9d8b2ad) )
	ROM_LOAD( "15.9k", 0x2000, 0x2000, CRC(dc126df9) SHA1(368efb36bf197e3eac23ef543e25e9f4efba785e) )
	ROM_LOAD( "16.9m", 0x4000, 0x2000, CRC(574a172d) SHA1(7dee073b5c8ff5825062e30cff29343dc767daaa) )

	ROM_REGION( 0x4000, "gfx3", 0 )
	ROM_LOAD( "17.9n", 0x0000, 0x2000, CRC(a4c4e4ce) SHA1(f8b0a8dfab972e23f268c69fd9ef30fc80f62533) )
	ROM_LOAD( "18.9r", 0x2000, 0x2000, CRC(9d75b104) SHA1(062884fdca9f705f555b828aff136d8f52fbf6eb) )

	ROM_REGION( 0x4000, "user1", 0 )
	ROM_LOAD( "13.7r", 0x0000, 0x2000, CRC(0acb8c61) SHA1(0ca3b07b21501d4c8feda2b4295f534da2a1c745) )
	ROM_LOAD( "12.7n", 0x2000, 0x2000, CRC(ae8d6ed2) SHA1(92d6484040a9b82f77b92f3dd452333fd618eb0d) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "9f.bin", 0x0000, 0x0100, CRC(ff71b27f) SHA1(6aad2bd2be997595a05ddb81d24df8fe1435910b) )
ROM_END

GAME( 1985, ksayakyu, 0, ksayakyu, ksayakyu, ksayakyu_state, empty_init, ORIENTATION_FLIP_Y, "Taito Corporation", "Kusayakyuu", MACHINE_SUPPORTS_SAVE )
