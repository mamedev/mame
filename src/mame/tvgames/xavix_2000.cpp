// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************/

/* (XaviX 2000 type CPU) hardware titles (2nd XaviX generation?)

   these use the SSD 2000 NEC 85605-621 type CPU

   This CPU type adds extra opcodes that don't appear to be present in the 97/98 types
   It does not appear to support the bitmap modes or 16-bit ROMs found in the 2002 type

   It is possible some games in the regular xavix.cpp drivers use this type, there
   does not appear to be a way to tell unless the extra opcodes are used or the CPU
   has been decapped

*/

#include "emu.h"
#include "xavix_2002.h"

// #define VERBOSE 1
#include "logmacro.h"

static INPUT_PORTS_START( xavix )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x00, "IN0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x00, "IN1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("AN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN3")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN4")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN5")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN7")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MOUSE0X")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("MOUSE0Y")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("MOUSE1X")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("MOUSE1Y")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("REGION") // PAL/NTSC flag
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM )
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_i2c )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
INPUT_PORTS_END


static INPUT_PORTS_START( epo_ebox )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // select
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // back
	// 04/08 not used for buttons?
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
INPUT_PORTS_END


static INPUT_PORTS_START( epo_bowl )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
INPUT_PORTS_END

static INPUT_PORTS_START( duelmast )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
INPUT_PORTS_END


static INPUT_PORTS_START( epo_sdb )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("MOUSE0X")
	PORT_BIT( 0xff, 0x01, IPT_AD_STICK_X ) PORT_MINMAX(0x01, 0xfe) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(1)
	PORT_MODIFY("MOUSE0Y")
	PORT_BIT( 0xff, 0x01, IPT_AD_STICK_Y ) PORT_MINMAX(0x01, 0xfe) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1)
	PORT_MODIFY("MOUSE1X")
	PORT_BIT( 0xff, 0x01, IPT_AD_STICK_X ) PORT_MINMAX(0x01, 0xfe) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(2)
	PORT_MODIFY("MOUSE1Y")
	PORT_BIT( 0xff, 0x01, IPT_AD_STICK_Y ) PORT_MINMAX(0x01, 0xfe) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(2)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( ttv_lotr )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_i2c_lotr_state::unknown_random_r))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_i2c_lotr_state::unknown_random_r))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
INPUT_PORTS_END

static INPUT_PORTS_START( epo_hamc )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_epo_hamc_state::unknown_random_r))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xavix_epo_hamc_state::unknown_random_r))
INPUT_PORTS_END

static INPUT_PORTS_START( ttv_mx )
	PORT_INCLUDE(xavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // Accel
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // Brake
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Pause")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Motion Up") // you tilt the device, but actual inputs are digital
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Motion Down")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Motion Left")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Motion Right")
INPUT_PORTS_END


void xavix_state::xavix2000(machine_config &config)
{
	xavix(config);

	XAVIX2000(config.replace(), m_maincpu, MAIN_CLOCK);
	set_xavix_cpumaps(config);

	m_palette->set_entries(512);
}

void xavix_state::xavix2000_4mb(machine_config &config)
{
	xavix2000(config);
	m_maincpu->set_addrmap(6, &xavix_state::xavix_4mb_extbus_map);
}

void xavix_state::xavix2000_nv(machine_config &config)
{
	xavix2000(config);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
}

void xavix_state::xavix2000_4mb_nv(machine_config &config)
{
	xavix2000_nv(config);
	m_maincpu->set_addrmap(6, &xavix_state::xavix_4mb_extbus_map);
}


void xavix_2000_nv_sdb_state::xavix2000_nv_sdb(machine_config &config)
{
	xavix2000_4mb_nv(config);

	m_anport->read_0_callback().set(FUNC(xavix_2000_nv_sdb_state::sdb_anport0_r));
	m_anport->read_1_callback().set(FUNC(xavix_2000_nv_sdb_state::sdb_anport1_r));
	m_anport->read_2_callback().set(FUNC(xavix_2000_nv_sdb_state::sdb_anport2_r));
	m_anport->read_3_callback().set(FUNC(xavix_2000_nv_sdb_state::sdb_anport3_r));
}

void xavix_i2c_state::xavix2000_i2c_24c08(machine_config &config)
{
	xavix2000(config);
	I2C_24C08(config, "i2cmem", 0);
}

void xavix_i2c_state::xavix2000_i2c_24c08_4mb(machine_config &config)
{
	xavix2000_i2c_24c08(config);
	m_maincpu->set_addrmap(6, &xavix_i2c_state::xavix_4mb_extbus_map);
}


void xavix_i2c_state::xavix2000_i2c_24c04(machine_config &config)
{
	xavix2000(config);

	I2C_24C04(config, "i2cmem", 0);
}

void xavix_i2c_state::xavix2000_i2c_24c04_2mb(machine_config &config)
{
	xavix2000_i2c_24c04(config);
	m_maincpu->set_addrmap(6, &xavix_i2c_state::xavix_2mb_extbus_map);
}

void xavix_i2c_state::xavix2000_i2c_24c04_4mb(machine_config &config)
{
	xavix2000_i2c_24c04(config);
	m_maincpu->set_addrmap(6, &xavix_i2c_state::xavix_4mb_extbus_map);
}


void xavix_i2c_state::xavix2000_i2c_24c02(machine_config &config)
{
	xavix2000(config);

	I2C_24C02(config, "i2cmem", 0);
}




ROM_START( epo_sdb )
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("superdashball.bin", 0x000000, 0x400000, CRC(a004a764) SHA1(47a96822d4d7d6a0f6be5cd729c3747dbab65979) )
ROM_END

ROM_START( epo_ebox )
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("exciteboxing.bin", 0x000000, 0x400000, CRC(e25ae4f5) SHA1(7f7b613f0ab8f43f5cad0d13de538921e77cae9c) )
ROM_END

ROM_START( epo_bowl )
	ROM_REGION(0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("bowling.bin", 0x000000, 0x200000, CRC(d34f8d9e) SHA1(ebe3792172dc43904b9226beb27f1da89d2388cc) )
ROM_END

ROM_START( epo_golf ) // GLFJ MAIN-03
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("golf.bin", 0x000000, 0x400000, CRC(d1f231cf) SHA1(9421836a6bc4af9ee1fc7a402d62b2fb4dbcdefc) )
ROM_END

ROM_START( epo_hamc ) // ET158 MB REV.0
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD( "hamster.u1", 0x000000, 0x400000, CRC(b1177813) SHA1(ed01096ebb63b72267ad7e0b2115224bbab64011) )
ROM_END

ROM_START( ban_omt ) // OMTJ MAIN-07
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("otmj.bin", 0x000000, 0x400000, CRC(1c1dc6fb) SHA1(d0cf1345b765d66ca9a0870ee6d0e3ccd84a8c0b) )
ROM_END

ROM_START( ttv_sw )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "jedi.bin", 0x000000, 0x800000, CRC(51cae5fd) SHA1(1ed8d556f31b4182259ca8c766d60c824d8d9744) )
ROM_END

ROM_START( ttv_swj )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "lightsaber.bin", 0x000000, 0x800000, CRC(a5c22ed0) SHA1(406f0bccb01cd4a26fe4a5675d7ebecc78c58147) )
ROM_END


ROM_START( ttv_lotr )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "lotr.bin", 0x000000, 0x800000, CRC(a034ecd5) SHA1(264a9d4327af0a075841ad6129db67d82cf741f1) )
ROM_END

ROM_START( ttv_mx )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "mxdirtrebel.bin", 0x000000, 0x800000, CRC(e64bf1a1) SHA1(137f97d7d857697a13e0c8984509994dc7bc5fc5) )
ROM_END

ROM_START( tom_jump )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "tom_jump.bin", 0x000000, 0x800000, CRC(20bf5c17) SHA1(bca7535baa6a54ad3ee0929bd3b74a22cb5139da) )
ROM_END


ROM_START( drgqst )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "dragonquest.bin", 0x000000, 0x800000, CRC(3d24413f) SHA1(1677e81cedcf349de7bf091a232dc82c6424efba) )
ROM_END

ROM_START( epo_mini )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "minimoni.u1", 0x000000, 0x400000, CRC(2adb01ee) SHA1(987218b6799195ba15adf39885c1d177c381ec26) )
ROM_END

ROM_START( tak_chq )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "choroq.u2", 0x000000, 0x400000, CRC(ffd2eb95) SHA1(a30884da5554483ebfd0009cf5dd1768be8a99cb) )
ROM_END

ROM_START( ban_onep )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("onepiece.bin", 0x000000, 0x800000, CRC(c5cb5a5f) SHA1(db85f6cc48d77c5a4967b9b8e2999167e3dfc8c8) )
ROM_END

ROM_START( duelmast )
	ROM_REGION(0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("duelmasters.u4", 0x000000, 0x200000, CRC(2f11fcd7) SHA1(d8849c74833e77b8b309e845523f2cdc7ac68054) )
ROM_END

ROM_START( tom_dpgm )
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("disney.bin", 0x000000, 0x400000, CRC(1dc181b3) SHA1(fa30069d17705f27e4ff45e7f6ccf06986e138f3) )
ROM_END

ROM_START( epo_es2j ) // ES2J MAIN-01  2005 date on PCB, 2006 ingame
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("es2j.u3", 0x000000, 0x400000, CRC(840aecb1) SHA1(ad52449ffc13af5f4c67b2c3cf438e7ecd80b9fb) )
ROM_END



// doesn't use extra opcodes?
// K.O.しようぜ！エキサイトボクシング
CONS( 2002, epo_ebox, 0, 0, xavix2000_4mb_nv,        epo_ebox,    xavix_state,             init_xavix, "Epoch / SSD Company LTD",       "Excite Boxing (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND ) // doesn't use XaviX2000 extra opcodes, but had that type of CPU

// die not confirmed, but uses extra opcodes.  (hangs on title screen due to combination of freq_timer_done nested interrupts tripping, and waiting on bits in input ports to change
// ストライクきめるぜ！ エキサイトボウリング
CONS( 2002, epo_bowl, 0, 0, xavix2000_i2c_24c04_2mb, epo_bowl,    xavix_i2c_state,         init_xavix, "Epoch / SSD Company LTD",       "Excite Bowling (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// スーパーショット！ エキサイトゴルフ
// needs timer irq hack to boot, fails to draw main menu properly (buggy xavix2000 opcodes?)  (2002 date on PCB, 2003 ingame)
CONS( 2003, epo_golf, 0,       0, xavix2000_i2c_24c04_4mb, ttv_lotr,   xavix_i2c_lotr_state, init_no_timer, "Epoch / SSD Company LTD",       "Super Shot! Excite Golf (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// とっとこハム太郎 ハムハム大サーカス！
CONS( 2002, epo_hamc,  0,      0, xavix2000_4mb,       epo_hamc,   xavix_epo_hamc_state, init_xavix,    "Epoch / SSD Company LTD",       "Tottoko Hamtaro - Ham Ham Dai Circus! (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// ミニモニ。パーティ！リズムでぴょん！
// needs timer irq hack to boot
CONS( 2003, epo_mini, 0,       0, xavix2000_i2c_24c08_4mb, ttv_lotr,   xavix_i2c_lotr_state, init_no_timer, "Epoch / SSD Company LTD",        "mini-moni Party! Rhythm de Pyon! (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// カードスキャン!　エキサイトステージ サッカー日本代表チーム
CONS( 2006, epo_es2j,   0,     0,  xavix2000_4mb,      xavix,      xavix_state,          init_xavix,    "Epoch / SSD Company LTD",        "Card Scan! Excite Stage Soccer Nippon Daihyou Team (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )


// takes a long time to boot to a card scanner error
// This is a product in the Duel Masters line called Duel Station; the boot up screen calls it Duel Station, title logo is Duel Masters
// It also has a cartridge slot in addition to the card scanner and at least 1 ROM cartridge was produced "デュエルマスターズ　デュエルステーション専用カートリッジ　Ver.1"
// デュエルステーション
CONS( 2003, duelmast, 0, 0, xavix2000_i2c_24c04_2mb, duelmast,    xavix_duelmast_state,    init_xavix, "Takara / SSD Company LTD",      "Duel Masters: Duel Station (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// スーパーダッシュボール
CONS( 2004, epo_sdb,  0, 0, xavix2000_nv_sdb,    epo_sdb,     xavix_2000_nv_sdb_state, init_xavix, "Epoch / SSD Company LTD",       "Super Dash Ball (Japan)",  MACHINE_IMPERFECT_SOUND )

CONS( 2005, ttv_sw,   0,      0, xavix2000_i2c_24c02, ttv_lotr,    xavix_i2c_lotr_state,    init_xavix, "Tiger / SSD Company LTD",       "Star Wars Saga Edition - Lightsaber Battle Game", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
CONS( 2005, ttv_swj,  ttv_sw, 0, xavix2000_i2c_24c02, ttv_lotr,    xavix_i2c_lotr_state,    init_xavix, "Tomy / SSD Company LTD",        "Star Wars Saga Edition - Lightsaber Battle Game (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
CONS( 2005, ttv_lotr, 0,      0, xavix2000_i2c_24c02, ttv_lotr,    xavix_i2c_lotr_state,    init_xavix, "Tiger / SSD Company LTD",       "Lord of the Rings - Warrior of Middle-Earth", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
CONS( 2005, ttv_mx,   0,      0, xavix2000_i2c_24c04, ttv_mx,      xavix_i2c_state,         init_xavix, "Tiger / SSD Company LTD",       "MX Dirt Rebel", MACHINE_IMPERFECT_SOUND )

// テレビで遊び隊　韋駄天翔 激走 韋駄天バトル  - seems to be based on the same engine at ttv_mx and has an almost identical controller, but not exactly the same game
CONS( 2005, tom_jump,   0,    0, xavix2000_i2c_24c04, ttv_mx,      xavix_i2c_state,         init_xavix, "Tomy / SSD Company LTD",        "IDATEN Jump: Gekisou IDATEN Battle (Japan)", MACHINE_IMPERFECT_SOUND )

// 剣神ドラゴンクエスト 甦りし伝説の剣
CONS( 2003, drgqst,   0,      0, xavix2000_i2c_24c08, ttv_lotr,    xavix_i2c_lotr_state,    init_xavix, "Square Enix / SSD Company LTD", "Kenshin Dragon Quest: Yomigaerishi Densetsu no Ken (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// チョロＱビュンビュンレーサー
// crashes whenever a CPU car reaches a corner - see map
CONS( 2003, tak_chq,  0,      0, xavix2000_i2c_24c04_4mb, xavix_i2c,   xavix_i2c_state,         init_xavix, "Takara / SSD Company LTD",      "Choro-Q Byun Byun Racer (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// hangs after starting a game, check why
// Let’s！TV プレイ　体感格闘ワンピースパンチバトル 　～海賊王にキミがなる！～
CONS( 2004, ban_onep, 0, 0, xavix2000_i2c_24c04, ttv_lotr,    xavix_i2c_lotr_state, init_xavix, "Bandai / SSD Company LTD",         "Let's! TV Play Taikan Kakutou One Piece Punch Battle - Kaizoku Ou ni Kimi ga Naru (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// Let’s！TV プレイ　闘印奥義　 陰陽大戦記～目指せ最強闘神士～
// stalls unless timers are disabled like epo_mini / epo_golf, 2004 date on PCB, 2005 ingame
CONS( 2005, ban_omt,  0, 0, xavix2000_i2c_24c04_4mb, ttv_lotr,    xavix_i2c_lotr_state, init_no_timer, "Bandai / SSD Company LTD",         "Let's! TV Play Touin Ougi Onmyou Taisenki: Mezase Saikyou Toushinshi (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// ディズニープリンセス　キラキラ魔法のレッスン
CONS( 2004, tom_dpgm, 0, 0, xavix2000_i2c_24c08_4mb, ttv_lotr,    xavix_i2c_lotr_state, init_xavix, "Tomy / SSD Company LTD",         "Disney Princess Kirakira Mahou no Lesson (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
