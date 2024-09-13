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
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, read_sda)
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
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, read_sda)
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
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, read_sda)
INPUT_PORTS_END


static INPUT_PORTS_START( epo_sdb )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("MOUSE0X")
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(1)
	PORT_MODIFY("MOUSE0Y")
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1)
	PORT_MODIFY("MOUSE1X")
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(2)
	PORT_MODIFY("MOUSE1Y")
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(2)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( ttv_lotr )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(xavix_i2c_lotr_state, camera_r)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(xavix_i2c_lotr_state, camera_r)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, read_sda)
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

void xavix_state::xavix2000_nv(machine_config &config)
{
	xavix2000(config);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
}

void xavix_2000_nv_sdb_state::xavix2000_nv_sdb(machine_config &config)
{
	xavix2000_nv(config);

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

void xavix_i2c_state::xavix2000_i2c_24c04(machine_config &config)
{
	xavix2000(config);

	I2C_24C04(config, "i2cmem", 0);
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

ROM_START( ttv_sw )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "jedi.bin", 0x000000, 0x800000, CRC(51cae5fd) SHA1(1ed8d556f31b4182259ca8c766d60c824d8d9744) )
ROM_END

ROM_START( ttv_lotr )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "lotr.bin", 0x000000, 0x800000, CRC(a034ecd5) SHA1(264a9d4327af0a075841ad6129db67d82cf741f1) )
ROM_END

ROM_START( ttv_mx )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "mxdirtrebel.bin", 0x000000, 0x800000, CRC(e64bf1a1) SHA1(137f97d7d857697a13e0c8984509994dc7bc5fc5) )
ROM_END

ROM_START( drgqst )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "dragonquest.bin", 0x000000, 0x800000, CRC(3d24413f) SHA1(1677e81cedcf349de7bf091a232dc82c6424efba) )
ROM_END

ROM_START( ban_onep )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("onepiece.bin", 0x000000, 0x800000, CRC(c5cb5a5f) SHA1(db85f6cc48d77c5a4967b9b8e2999167e3dfc8c8) )
ROM_END

ROM_START( duelmast )
	ROM_REGION(0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("duelmasters.u4", 0x000000, 0x200000, CRC(2f11fcd7) SHA1(d8849c74833e77b8b309e845523f2cdc7ac68054) )
ROM_END


// doesn't use extra opcodes?
CONS( 2002, epo_ebox, 0, 0, xavix2000_nv,        epo_ebox,    xavix_state,             init_xavix, "Epoch / SSD Company LTD",       "Excite Boxing (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND ) // doesn't use XaviX2000 extra opcodes, but had that type of CPU
 // die not confirmed, but uses extra opcodes.  (hangs on title screen due to combination of freq_timer_done nested interrupts tripping, and waiting on bits in input ports to change
CONS( 2002, epo_bowl, 0, 0, xavix2000_i2c_24c04, epo_bowl,    xavix_i2c_state,         init_xavix, "Epoch / SSD Company LTD",       "Excite Bowling (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// takes a long time to boot to a card scanner error
// This is a product in the Duel Masters line called Duel Station; the boot up screen calls it Duel Station, title logo is Duel Masters
// It also has a cartridge slot in addition to the card scanner and at least 1 ROM cartridge was produced "デュエルマスターズ　デュエルステーション専用カートリッジ　Ver.1"
CONS( 2003, duelmast, 0, 0, xavix2000_i2c_24c04, duelmast,    xavix_duelmast_state,    init_xavix, "Takara / SSD Company LTD",      "Duel Masters: Duel Station (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

CONS( 2004, epo_sdb,  0, 0, xavix2000_nv_sdb,    epo_sdb,     xavix_2000_nv_sdb_state, init_xavix, "Epoch / SSD Company LTD",       "Super Dash Ball (Japan)",  MACHINE_IMPERFECT_SOUND )

CONS( 2005, ttv_sw,   0, 0, xavix2000_i2c_24c02, ttv_lotr,    xavix_i2c_lotr_state,    init_xavix, "Tiger / SSD Company LTD",       "Star Wars Saga Edition - Lightsaber Battle Game", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
CONS( 2005, ttv_lotr, 0, 0, xavix2000_i2c_24c02, ttv_lotr,    xavix_i2c_lotr_state,    init_xavix, "Tiger / SSD Company LTD",       "Lord Of The Rings - Warrior of Middle-Earth", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
CONS( 2005, ttv_mx,   0, 0, xavix2000_i2c_24c04, ttv_mx,      xavix_i2c_state,         init_xavix, "Tiger / SSD Company LTD",       "MX Dirt Rebel", MACHINE_IMPERFECT_SOUND )
CONS( 2003, drgqst,   0, 0, xavix2000_i2c_24c08, ttv_lotr,    xavix_i2c_lotr_state,    init_xavix, "Square Enix / SSD Company LTD", "Kenshin Dragon Quest: Yomigaerishi Densetsu no Ken", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// hangs after starting a game, check why
CONS( 2004, ban_onep, 0, 0, xavix2000_i2c_24c04, ttv_lotr,    xavix_i2c_lotr_state, init_xavix, "Bandai / SSD Company LTD",         "Let's! TV Play One Piece Punch Battle (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
