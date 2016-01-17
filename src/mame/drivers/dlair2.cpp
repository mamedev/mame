// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

Skeleton driver for Dragon's Lair II: Time Warp
by Leland

Notes:
- two irqs, timer (vector 0x20/4) and serial (vector 0x2c/4)

Some information from
http://www.dragons-lair-project.com/tech/pages/dl2.asp

* Service Mode:
  Press and hold the "Sword" button and "Service switch"
  (located inside coin door) - Release both buttons

* Rom version determines LD Image to use:
  ROM revision 2.00 works only with a Dragon's Lair II/Space Ace '91 proto disc,
  and an AMOA prototype board with DL2 BIOS ROM.
  ROM revision 2.xx works only with a Dragon's Lair II disc, serial number
  C-910-00001-00.  This is the original pressing of the laser disc.
  ROM revision 3.xx works with a Dragon's Lair II disc, serial number
  C-910-00002-00, which is the 2nd pressing of the laser disc.

* Coinage seems to be controlled by a PIC16C54 with an internal ROM. (Phil B)

* Space Ace (slightly modified from original) was also offered on this
  hardware as a conversion kit.  Known sets are included.

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"

#define MAIN_CLOCK XTAL_30MHz

class dlair2_state : public driver_device
{
public:
	dlair2_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"){ }

	// devices
	required_device<cpu_device> m_maincpu;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(dlair2_timer_irq);
	DECLARE_PALETTE_INIT(dlair2);

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
};

void dlair2_state::video_start()
{
}

UINT32 dlair2_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

static ADDRESS_MAP_START( dlair2_map, AS_PROGRAM, 8, dlair2_state )
	AM_RANGE(0x00000, 0xeffff) AM_RAM
	AM_RANGE(0xf0000, 0xfffff) AM_ROM AM_REGION("ipl", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dlair2_io, AS_IO, 8, dlair2_state )
//  AM_RANGE(0x020, 0x020) ICR
//  AM_RANGE(0x042, 0x043) sound related
//  AM_RANGE(0x061, 0x061) sound related
//  AM_RANGE(0x200, 0x203) i/o, coin, eeprom
//  AM_RANGE(0x2f8, 0x2ff) COM2
ADDRESS_MAP_END

static INPUT_PORTS_START( dlair2 )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
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

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
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
INPUT_PORTS_END

/*
static const gfx_layout charlayout =
{
    8,8,
    RGN_FRAC(1,1),
    1,
    { RGN_FRAC(0,1) },
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
    8*8
};

static GFXDECODE_START( dlair2 )
    GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
GFXDECODE_END
*/

void dlair2_state::machine_start()
{
}

void dlair2_state::machine_reset()
{
}

PALETTE_INIT_MEMBER(dlair2_state, dlair2)
{
}

INTERRUPT_GEN_MEMBER(dlair2_state::dlair2_timer_irq)
{
	device.execute().set_input_line_and_vector(0,HOLD_LINE,0x20/4);
}

static MACHINE_CONFIG_START( dlair2, dlair2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088 , MAIN_CLOCK/3)   /* Schematics show I8088 "max" CPU */
	MCFG_CPU_PROGRAM_MAP(dlair2_map)
	MCFG_CPU_IO_MAP(dlair2_io)
	MCFG_CPU_PERIODIC_INT_DRIVER(dlair2_state, dlair2_timer_irq, 60) // timer irq, TODO: timing

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(dlair2_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_PALETTE("palette")

//  MCFG_GFXDECODE_ADD("gfxdecode", "palette", dlair2)

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(dlair2_state, dlair2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( dlair2 )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "dl2_319.bin",     0x00000, 0x10000, CRC(e9453a1b) SHA1(eb1201abd0124f6edbabd49bec81af827369cb2c) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "C-910-00002-00", 0, NO_DUMP )
ROM_END

ROM_START( dlair2_319e )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "dl2euro3.19.bin", 0x00000, 0x10000, CRC(cc23ad9f) SHA1(24add8f03749dcc27b1b166dc2e5d346534a0088) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "C-910-00002-00", 0, NO_DUMP )
ROM_END

ROM_START( dlair2_319s )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "dl2-span.bin",    0x00000, 0x10000, CRC(4b9a811d) SHA1(6fe580f541305422f89edbbf475f7c5f17153738) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "dlair2_span", 0, NO_DUMP )
ROM_END

ROM_START( dlair2_318 )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "dl2_318.bin",     0x00000, 0x10000, CRC(64706492) SHA1(99c92572c59ce1206847a5363d3791196fccd742) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "C-910-00002-00", 0, NO_DUMP )
ROM_END

ROM_START( dlair2_317e )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "dl2euro3.17.bin", 0x00000, 0x10000, CRC(743f65a5) SHA1(45199983156c561b8e88c69bef454fd4042579bb) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "C-910-00002-00", 0, NO_DUMP )
ROM_END

ROM_START( dlair2_316e )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "dl2euro.bin",     0x00000, 0x10000, CRC(d68f1b13) SHA1(cc9ee307b4d3caba049be6226163c810cf89ab44) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "C-910-00002-00", 0, NO_DUMP )
ROM_END

ROM_START( dlair2_315 )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "dl2_315.rom",     0x00000, 0x10000, CRC(13ec0600) SHA1(9366dfac4508c4a723d688016b8cddb57aa6f5f1) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "C-910-00002-00", 0, NO_DUMP )
ROM_END

ROM_START( dlair2_315s )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "315pi.bin",       0x00000, 0x10000, CRC(75d8861a) SHA1(56ab31a760f43f98fa40396ee7d7af7ce982d28d) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "dlair2_span", 0, NO_DUMP )
ROM_END

ROM_START( dlair2_314 )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "dl2_314.bin",     0x00000, 0x10000, CRC(af92b612) SHA1(a0b986fa8a0f2206beedf1dcaed4d108599947ff) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "C-910-00002-00", 0, NO_DUMP )
ROM_END

ROM_START( dlair2_312 )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "312.bin",         0x00000, 0x10000, CRC(c842be6b) SHA1(bf548ea3c6e98cd93f79408c3b9f0e1e22cc8bd1) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "C-910-00002-00", 0, NO_DUMP )
ROM_END

ROM_START( dlair2_300 )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "dl2_300.bin",     0x00000, 0x10000, CRC(dec4f2e3) SHA1(fd96378c78df4aacd4b2190823ec5c1591199d44) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "C-910-00002-00", 0, NO_DUMP )
ROM_END

ROM_START( dlair2_211 )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "dl2_211.bin",     0x00000, 0x10000, CRC(9f2660a3) SHA1(bf35356aab0138f86e6ea18c7bcf4f3f3c428d98) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "C-910-00001-00", 0, NO_DUMP )
ROM_END

ROM_START( dlair2_200 )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "dl2_amoa_proto_9-8_2e7f.bin",      0x00000, 0x10000,CRC(b41bad8d) SHA1(c9d594f94c349d83f07c1f4730bfde371834263d) )

	ROM_REGION( 0x2000, "bios", 0 ) /* BIOS for proto board */
	ROM_LOAD( "dl2_amoa_proto_bios_mod_56ee.bin",0x00000, 0x2000, CRC(1fc21576) SHA1(dc5443f6a8d80ec8148314244f05ac0290e380ea) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "dl2-sa91_proto", 0, NO_DUMP )
ROM_END

ROM_START( spacea91 )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "ace.dat",         0x00000, 0x10000, CRC(de93a213) SHA1(1c95d5f45292f08149d749e1f7b5d9409d3a266e) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "spaceace91", 0, NO_DUMP )
ROM_END

ROM_START( spacea91_13e )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "sa91euro1.3.bin", 0x00000, 0x10000, CRC(27dd0486) SHA1(8a57510b466381d9962e5397d89a7a3e73d757b0) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "spaceace91", 0, NO_DUMP )
ROM_END


GAME( 1991, dlair2,       0,        dlair2,  dlair2, driver_device,  0,       ROT0, "Leland",      "Dragon's Lair 2: Time Warp (US v3.19)", MACHINE_IS_SKELETON )
GAME( 1991, dlair2_319e,  dlair2,   dlair2,  dlair2, driver_device,  0,       ROT0, "Leland",      "Dragon's Lair 2: Time Warp (Euro v3.19)", MACHINE_IS_SKELETON )
GAME( 1991, dlair2_319s,  dlair2,   dlair2,  dlair2, driver_device,  0,       ROT0, "Leland",      "Dragon's Lair 2: Time Warp (Spanish v3.19)", MACHINE_IS_SKELETON )
GAME( 1991, dlair2_318,   dlair2,   dlair2,  dlair2, driver_device,  0,       ROT0, "Leland",      "Dragon's Lair 2: Time Warp (US v3.18)", MACHINE_IS_SKELETON )
GAME( 1991, dlair2_317e,  dlair2,   dlair2,  dlair2, driver_device,  0,       ROT0, "Leland",      "Dragon's Lair 2: Time Warp (Euro v3.17)", MACHINE_IS_SKELETON )
GAME( 1991, dlair2_316e,  dlair2,   dlair2,  dlair2, driver_device,  0,       ROT0, "Leland",      "Dragon's Lair 2: Time Warp (Euro v3.16)", MACHINE_IS_SKELETON )
GAME( 1991, dlair2_315,   dlair2,   dlair2,  dlair2, driver_device,  0,       ROT0, "Leland",      "Dragon's Lair 2: Time Warp (US v3.15)", MACHINE_IS_SKELETON )
GAME( 1991, dlair2_315s,  dlair2,   dlair2,  dlair2, driver_device,  0,       ROT0, "Leland",      "Dragon's Lair 2: Time Warp (Spanish v3.15)", MACHINE_IS_SKELETON )
GAME( 1991, dlair2_314,   dlair2,   dlair2,  dlair2, driver_device,  0,       ROT0, "Leland",      "Dragon's Lair 2: Time Warp (US v3.14)", MACHINE_IS_SKELETON )
GAME( 1991, dlair2_312,   dlair2,   dlair2,  dlair2, driver_device,  0,       ROT0, "Leland",      "Dragon's Lair 2: Time Warp (Euro v3.12)", MACHINE_IS_SKELETON )
GAME( 1991, dlair2_300,   dlair2,   dlair2,  dlair2, driver_device,  0,       ROT0, "Leland",      "Dragon's Lair 2: Time Warp (US v3.00)", MACHINE_IS_SKELETON )
GAME( 1991, dlair2_211,   dlair2,   dlair2,  dlair2, driver_device,  0,       ROT0, "Leland",      "Dragon's Lair 2: Time Warp (US v2.11)", MACHINE_IS_SKELETON )
GAME( 1991, dlair2_200,   dlair2,   dlair2,  dlair2, driver_device,  0,       ROT0, "Leland",      "Dragon's Lair 2: Time Warp (US v2.00, AMOA prototype)", MACHINE_IS_SKELETON )
GAME( 1991, spacea91,     0,        dlair2,  dlair2, driver_device,  0,       ROT0, "Leland",      "Space Ace (DL2 Conversion) (US v1.3)", MACHINE_IS_SKELETON )
GAME( 1991, spacea91_13e, spacea91, dlair2,  dlair2, driver_device,  0,       ROT0, "Leland",      "Space Ace (DL2 Conversion) (Euro v1.3)", MACHINE_IS_SKELETON )
