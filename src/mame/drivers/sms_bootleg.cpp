// license:BSD-3-Clause
// copyright-holders:David Haywood

/*

----------------------------------
for Sono Corp Japan
----------------------------------

this is an SMS multi-game bootleg with 32 games



there are also empty k9/k10/k11/k12 positions, but they were clearly never used.


Games ordered by MENU

Super Bubble    ( port08_w 00 )
Tetris   << ??  ( port08_w ac )
Wonder Boy << ??  ( port08_w cc )
Alex Kidd ( port08_w 07 )
Super Mario  << ?? ( port08_w 3c )
Hello Kang Si ( port08_w 0d )
Solomon Key ( port08_w 02 )
Buk Doo Gun ( port08_w 0e )

Invaders  << ?? ( port08_w 1c )
Galaxian ( port08_w 14 )
Galaga  << ?? ( port08_w 6c )
Flicky  ( port08_w 04 )
Teddy Boy ( port08_w 25 )
Ghost House << ?? ( port08_w 4c )
Bomb Jack << ?? ( port08_w 5c )
Kings Vally ( port08_w 24 )

Pippols  ( port08_w 34 )
Dragon Story ( port08_w 05 )
Spy vs Spy ( port08_w 15 )
PitFall II ( port08_w 35 )
Drol ( port08_w 06 )
Pit Pot ( port08_w 16 )
Hyper Sport ( port08_w 26 )
Super Tank ( port08_w 36 )

Congo Bongo ( port08_w 23 )
Circus ( port08_w 33 )
Road Fighter << ?? ( port08_w 9c )
Astro << ?? ( port08_w 2c )
Goonies << ?? ( port08_w 7c )
Road Runner I << ?? ( port08_w 8c )
Masic Tree ( port08_w 03 )
Mouse ( port08_w 13 )


list reordered based on ROMs and banking  (the low 4 bits are the ROM select, the upper 4 bits are the bank select)
you can clearly see

ROM K2.bin / ROM K1.bin
Super Bubble     ( port08_w 00 ) (8 banks - 2 roms)

ROM K3.bin
Solomon Key      ( port08_w 02 ) (all 4 banks)

ROM K4.bin
Masic Tree       ( port08_w 03 )
Mouse            ( port08_w 13 )
Congo Bongo      ( port08_w 23 )
Circus           ( port08_w 33 )

ROM K5.bin
Flicky           ( port08_w 04 )
Galaxian         ( port08_w 14 )
Kings Vally      ( port08_w 24 )
Pippols          ( port08_w 34 )

ROM K6.bin
Dragon Story     ( port08_w 05 )
Spy vs Spy       ( port08_w 15 )
Teddy Boy        ( port08_w 25 )
PitFall II       ( port08_w 35 )

ROM K7.bin
Drol             ( port08_w 06 )
Pit Pot          ( port08_w 16 )
Hyper Sport      ( port08_w 26 )
Super Tank       ( port08_w 36 )

ROM K8.bin
Alex Kidd        ( port08_w 07 ) (all 4 banks)

port08_w x8 would be K9 (unpopulated)
port08_w x9 would be K10 (unpopulated)
port08_w xa would be K11 (unpopulated)
port08_w xb would be K12 (unpopulated)


ROM rom4.bin
Invaders  << ??      ( port08_w 1c )
Astro << ??          ( port08_w 2c )
Super Mario  << ??   ( port08_w 3c )
Ghost House << ??    ( port08_w 4c )
Bomb Jack << ??      ( port08_w 5c )
Galaga  << ??        ( port08_w 6c )
Goonies << ??        ( port08_w 7c )
Road Runner I << ??  ( port08_w 8c )
Road Fighter << ??   ( port08_w 9c )
Tetris   << ??       ( port08_w ac )
Wonder Boy << ??     ( port08_w cc )

ROM rom3.bin
Hello Kang Si    ( port08_w 0d )

ROM rom2.bin
Buk Doo Gun      ( port08_w 0e )

----------------------------------
for TV Tuning set
----------------------------------

Games ordered by MENU

Tri-Formation ( port08_w 01 )
Tetris ( port08_w 08 )
Wonderboy ( port08_w 0c )
Alex Kidd ( port08_w 0b )
Super Mario ( port08_w 64 )
Hello Kang Si ( port08_w 0d )
Solomon's Key  ( port08_w 02 )
Buk Doo Gun ( port08_w 0e )

Invaders ( port08_w 63 )
Galaxian ( port08_w 55 )
Galaga ( port08_w 75 )
Flicky ( port08_w 45 )
Teddy Boy ( port08_w 68 )
Ghost House ( port08_w 74 )
Bomb Jack ( port08_w 65 )
Kings Vally ( port08_w 46 )

Pippols ( port08_w 56 )
Dragon Story ( port08_w 47 )
Spy Vs Spy ( port08_w 57 )
Pitfall 2 ( port08_w 78 )
Drol ( port08_w 49 )
Pit Pot ( port08_w 59 )
Hyper Sport ( port08_w 4a )
Super Tank ( port08_w 5a )

Congo Bongo ( port08_w 44 )
Circus ( port08_w 54 )
Road Fighter ( port08_w 67 )
Astro ( port08_w 73 )
Goonies ( port08_w 66 )
Road Runner I ( port08_w 76 )
Maisc Tree ( port08_w 43 )
Mouse ( port08_w 53 )


Games ordered by ROM

ROM 01.K1
Tri-Formation ( port08_w 01 )

ROM 02.K2
Game Menus

ROM 03.K3
Solomon's Key  ( port08_w 02 )

ROM 04.K4
Maisc Tree ( port08_w 43 )
Mouse ( port08_w 53 ) [Hustle Chumy]
Invaders ( port08_w 63 )
Astro ( port08_w 73 )

ROM 05.K5
Congo Bongo ( port08_w 44 )
Circus ( port08_w 54 )
Super Mario ( port08_w 64 ) [Super Boy 1]
Ghost House ( port08_w 74 )

ROM 06.K6
Flicky ( port08_w 45 )
Galaxian ( port08_w 55 )
Bomb Jack ( port08_w 65 )
Galaga ( port08_w 75 )

ROM 07.K7
Kings Vally ( port08_w 46 )
Pippols ( port08_w 56 )
Goonies ( port08_w 66 )
Road Runner I ( port08_w 76 ) [Lode Runner]

ROM 08.K8
Dragon Story ( port08_w 47 )
Spy Vs Spy ( port08_w 57 )
Road Fighter ( port08_w 67 )

ROM 09.K9
Tetris ( port08_w 08 )
Teddy Boy ( port08_w 68 )
Pitfall 2 ( port08_w 78 )

ROM 10.K10
Drol ( port08_w 49 )
Pit Pot ( port08_w 59 )

ROM 11.K11
Hyper Sport ( port08_w 4a ) [Hyper Sports 2]
Super Tank ( port08_w 5a )

ROM 12.K12
Alex Kidd ( port08_w 0b )

ROM 13.ROM4
Wonderboy ( port08_w 0c )

ROM 14.ROM3
Hello Kang Si ( port08_w 0d )

ROM 15.ROM2
Buk Doo Gun ( port08_w 0e )

----------------------------------

A Korean version has been seen too (unless this can be switched?)

*/

#include "emu.h"
#include "includes/sms_bootleg.h"

#include "cpu/z80/z80.h"
#include "speaker.h"



void smsbootleg_state::sms_supergame_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xfff7).ram();
//  AM_RANGE(0xfffc, 0xffff) AM_READWRITE(sms_mapper_r, sms_mapper_w)       /* Bankswitch control */
}

WRITE8_MEMBER(smsbootleg_state::port08_w)
{
//  logerror("port08_w %02x\n", data);
}

WRITE8_MEMBER(smsbootleg_state::port18_w)
{
	logerror("port18_w %02x\n", data);
}


void smsbootleg_state::sms_supergame_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x04, 0x04).nopr(); //AM_READ_PORT("IN0") // these
	map(0x08, 0x08).w(FUNC(smsbootleg_state::port08_w));
	map(0x14, 0x14).nopr(); //AM_READ_PORT("IN1") // seem to be from a coinage / timer MCU, changing them directly changes the credits / time value
	map(0x18, 0x18).w(FUNC(smsbootleg_state::port18_w));

	map(0x40, 0x7f).r(FUNC(smsbootleg_state::sms_count_r)).w(m_vdp, FUNC(sega315_5124_device::psg_w));
	map(0x80, 0x80).mirror(0x3e).rw(m_vdp, FUNC(sega315_5124_device::data_read), FUNC(sega315_5124_device::data_write));
	map(0x81, 0x81).mirror(0x3e).rw(m_vdp, FUNC(sega315_5124_device::control_read), FUNC(sega315_5124_device::control_write));

	map(0xdc, 0xdc).portr("IN2");
}



void smsbootleg_state::sms_supergame(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(10'738'635)/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &smsbootleg_state::sms_supergame_map);
	m_maincpu->set_addrmap(AS_IO, &smsbootleg_state::sms_supergame_io);

	config.m_minimum_quantum = attotime::from_hz(60);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	SCREEN(config, m_main_scr, SCREEN_TYPE_RASTER);
	m_main_scr->set_raw(XTAL(10'738'635)/2, \
			sega315_5124_device::WIDTH , sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH - 2, sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH + 256 + 10, \
			sega315_5124_device::HEIGHT_NTSC, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT + 224);
	m_main_scr->set_refresh_hz(XTAL(10'738'635)/2 / (sega315_5124_device::WIDTH * sega315_5124_device::HEIGHT_NTSC));
	m_main_scr->set_screen_update(FUNC(sms_state::screen_update_sms));

	SEGA315_5246(config, m_vdp, XTAL(10'738'635));
	m_vdp->set_screen(m_main_scr);
	m_vdp->set_is_pal(false);
	m_vdp->irq().set_inputline(m_maincpu, 0);
	m_vdp->pause().set(FUNC(sms_state::sms_pause_callback));
	m_vdp->add_route(ALL_OUTPUTS, "mono", 1.00);
}





static INPUT_PORTS_START( sms_supergame )
	PORT_START("PAUSE")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )// PORT_NAME(DEF_STR(Pause)) PORT_CODE(KEYCODE_1)
#if 0
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
#endif
	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void smsbootleg_state::init_sms_supergame()
{
	uint8_t* rom = memregion("maincpu")->base();
	size_t size = memregion("maincpu")->bytes();

	for (int i = 0; i < size; i++)
	{
		rom[i] ^= 0x80;
	}
}


ROM_START( smssgame )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "rom1.bin", 0x00000, 0x10000, CRC(0e1f258e) SHA1(9240dc0d01e3061c0c8807c07c0a1d033ebe9116) ) // yes, this rom is smaller (menu rom)
	ROM_LOAD( "rom2.bin",0x020000, 0x20000, CRC(c1478323) SHA1(27b524a234f072e81ef41fb89a5fff5617e9b951) ) // Buk Doo Sun
	ROM_LOAD( "rom3.bin",0x040000, 0x20000, CRC(96c8705d) SHA1(ba4f4af0cfdad1d63a08201ed186c79aea062b95) ) // ? Kung Fu game (Hello Kang Si?)
	// k12 unpopulated
	// k11 unpopulated
	// k10 unpopulated
	// k9 unpopulated
	ROM_LOAD( "k8.bin",  0x060000, 0x20000, CRC(eb1e8693) SHA1(3283cdcfc25f34a43f317093cd39e10a52bc3ae7) ) // Alex Kidd in Miracle World
	ROM_LOAD( "k7.bin",  0x080000, 0x20000, CRC(d24da417) SHA1(2ef5e55748e412157e55e7a62355fd66bf792d8e) ) // Drol / Pit Pot / Hyper Sports II / Super Tank
	ROM_LOAD( "k6.bin",  0x0a0000, 0x20000, CRC(d78ce5ba) SHA1(06065cec3865ff3a2bb2f56702b24427487964e2) ) // Dragon Story / Spy Vs Spy / Teddyboy Blues / Pitfall II
	ROM_LOAD( "k5.bin",  0x0c0000, 0x20000, CRC(a7b64d1c) SHA1(7c37ac3f37699c49492d4f4ea4e213670413041c) ) // Flicky / Galaxian / King's Valley / Pippos
	ROM_LOAD( "k4.bin",  0x0e0000, 0x20000, CRC(e5903942) SHA1(d0c02f4b37c8a02142868459af14ba8ed0340ccd) ) // Magical Tree / Chustle Chumy / Congo Bongo / Charlie Circus
	ROM_LOAD( "k3.bin",  0x100000, 0x20000, CRC(9bb92096) SHA1(3ca17b7a9aa20b97cac1f78ba13f70bed1b37463) ) // Solomon's Key
	ROM_LOAD( "k2.bin",  0x120000, 0x20000, CRC(a12439f4) SHA1(e957d4fe275e982bedef28af8cc2957da27dc512) ) // Final Bubble Bobble (1/2)
	ROM_LOAD( "k1.bin",  0x140000, 0x20000, CRC(dadffecd) SHA1(68ebb968539049a9e193da5200856b9f956f7e02) ) // Final Bubble Bobble (2/2)
	// this mask rom appears to be taken straight from an SMS multi-game cart, bank 0 of it is even a menu just for the games in this ROM! same style, so presumably the same developer (Seo Jin 1990 copyright)
	ROM_LOAD( "sg11004a 79st0086end 9045.rom4",0x180000, 0x080000, CRC(cdbfe86e) SHA1(83d6f261471dca20f8d2e33b9807d670e9b4eb9c) ) // Invaders, Astro, Super Mario, Ghost House, Bomb Jack, Galaga, Goonies, Road Runner I, Road Fighter, Tetris, Wonder Boy

	// there seems to be some kind of MCU for the timer?
ROM_END


ROM_START( smssgamea )
	ROM_REGION( 0x280000, "maincpu", 0 )
	ROM_LOAD( "02.k2",   0x000000, 0x10000, CRC(66ed320e) SHA1(e838cb98fbd295259707f8f7ce433b28baa846e3) ) // menu is here on this one
	ROM_LOAD( "01.k1",   0x020000, 0x20000, CRC(18fd8607) SHA1(f24fbe863e19b513369858bf1260355e92444071) ) // Tri-Formation
	ROM_LOAD( "03.k3",   0x040000, 0x20000, CRC(9bb92096) SHA1(3ca17b7a9aa20b97cac1f78ba13f70bed1b37463) ) // Solomon's Key
	ROM_LOAD( "04.k4",   0x060000, 0x20000, CRC(28f6f4a9) SHA1(87809d93b8393b3186672c217fa1dec8b152af16) ) // Maisc Tree, Mouse, Invaders, Astro
	ROM_LOAD( "05.k5",   0x080000, 0x20000, CRC(350591a4) SHA1(ceb3c4a0fc85c5fbc5a045e9c83c3e7ec4d535cc) ) // Congo Bongo, Circus, Super Mario, Ghost House
	ROM_LOAD( "06.k6",   0x0a0000, 0x20000, CRC(9c5e7cc7) SHA1(4613928e30b7faaa41d550fa41906e13a6059513) ) // Flicky, Galaxian, Bomb Jack, Galaga
	ROM_LOAD( "07.k7",   0x0c0000, 0x20000, CRC(8046a2c0) SHA1(c80298dd56db8c09cac5263e4c01a627ab1a4cda) ) // Kings Vally, Pippols, Goonies, Road Runner I
	ROM_LOAD( "08.k8",   0x0e0000, 0x20000, CRC(ee366e0f) SHA1(3770aa71372e7dbdfd357b239a0fbdf8880dc135) ) // Dragon Story, Spy Vs Spy, Road Fighter
	ROM_LOAD( "09.k9",   0x100000, 0x20000, CRC(50a66ef6) SHA1(8eb8d1a7ecca99d1722534be269a6264d49b9dd4) ) // Tetris, Teddy Boy, Pitfall 2
	ROM_LOAD( "10.k10",  0x120000, 0x10000, CRC(ca7ab2df) SHA1(11a85f03ec21d481c5cdfcfb749da20b8569d09a) ) // Drol, Pit Pot
	ROM_LOAD( "11.k11",  0x140000, 0x10000, CRC(b03b612f) SHA1(537b7d72e1e06e17db6206a37f2480c14f46b9fc) ) // Hyper Sports, Super Tank
	ROM_LOAD( "12.k12",  0x160000, 0x20000, CRC(eb1e8693) SHA1(3283cdcfc25f34a43f317093cd39e10a52bc3ae7) ) // Alex Kidd
	ROM_LOAD( "13.rom4", 0x180000, 0x20000, CRC(8767f1c9) SHA1(683cedb001e859c2c7ccde2571104f1eb9f09c2f) ) // Wonderboy
	ROM_LOAD( "14.rom3", 0x1a0000, 0x20000, CRC(889bb269) SHA1(0a92b339c19240bfea29ee24fee3e7d780b0cd5c) ) // Hello Kang Si
	ROM_LOAD( "15.rom2", 0x1c0000, 0x20000, CRC(c1478323) SHA1(27b524a234f072e81ef41fb89a5fff5617e9b951) ) // Buk Doo Gun
	ROM_FILL(            0x200000, 0x80000, 0xff) // ROM1 position not populated

	// there seems to be some kind of MCU for the timer?
ROM_END


// these haven't been set as clones because they contain different games
GAME( 199?, smssgame,  0,    sms_supergame, sms_supergame, smsbootleg_state, init_sms_supergame, ROT0, "Sono Corp Japan", "Super Game (Sega Master System Multi-game bootleg)", MACHINE_NOT_WORKING )
GAME( 1990, smssgamea, 0,    sms_supergame, sms_supergame, smsbootleg_state, init_sms_supergame, ROT0, "Seo Jin (TV-Tuning license)", "Super Game (Sega Master System Multi-game bootleg) (alt games)", MACHINE_NOT_WORKING ) // for German market?
