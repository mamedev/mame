// license:BSD-3-Clause
// copyright-holders:kazblox
/*
Space Warp(?)
it's completely unknown if this is actually Space Warp but it contains Galaxian-like GFX
ASCII shows "COPYRIGHT 1983", "CENTURY ELECTRONICS UK LTD" in swarpt7f.bin
gfx is like cosmos according to hap(cvs.c)
*/

#include "emu.h"
#include "cpu/s2650/s2650.h"

class spcwarp_state : public driver_device
{
public:
	spcwarp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	virtual void machine_start();
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
};

//INTERRUPT_GEN_MEMBER(spcwarp_state::spcwarp_main_cpu_interrupt)
//{
//	device.execute().set_input_line_vector(0, 0x03);
//}

static ADDRESS_MAP_START( spcwarp_map, AS_PROGRAM, 8, spcwarp_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( spcwarp )
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
INPUT_PORTS_END

void spcwarp_state::machine_start()
{
}

void spcwarp_state::machine_reset()
{
}

static MACHINE_CONFIG_START( spcwarp, spcwarp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, XTAL_14_31818MHz/16) // clock is a complete guess
	MCFG_CPU_PROGRAM_MAP(spcwarp_map)
	//MCFG_CPU_VBLANK_INT_DRIVER("screen", spcwarp_state, spcwarp_main_cpu_interrupt) // ???
	//MCFG_S2650_FLAG_HANDLER(WRITELINE(spcwarp_state, write_s2650_flag)) // ???

	/* video hardware */
	/* todo: this */

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	/* todo: this */

MACHINE_CONFIG_END

ROM_START( spcwarp )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "swarpt7f.bin", 0x0000, 0x1000, CRC(04d744e3) SHA1(db8218510052a05670cb0b722b73d3f10464788c) )
	ROM_LOAD( "swarpt7h.bin", 0x1000, 0x1000, CRC(34a36536) SHA1(bc438515618683b2a7c29637871ee00ed95ad7f8) )
/* ROMCMP reports "BADADDR            xxxxxx-xxxxx".  Observed data sequence repeated every 32 bytes */
	ROM_LOAD( "swarpt7m.bin", 0x2000, 0x1000, BAD_DUMP CRC(a2dff6c8) SHA1(d1c72848450dc5ff386dc94a26e4bf704ccc7121) )
/* Stripped "repaired" rom.  Was original rom supposed to be 0x1000 or 0x800? */
//  ROM_LOAD( "swarpt7m-repair.bin", 0x2000, 0x0800, CRC(109f95cf) SHA1(d99171ffd6639fec28966edaf7cce3a4df5e948d) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "swarpb1h.bin", 0x0000, 0x0800, CRC(6ee3b5f7) SHA1(8150f2ecd59d3a165c0541b550664c56d049edd5) )
	ROM_LOAD( "swarpb1k.bin", 0x0800, 0x0800, CRC(da4cee6b) SHA1(28b91381658f598fa62049489beee443232825c6) )
ROM_END

GAME( 1983, spcwarp,     0,        spcwarp,   spcwarp,   driver_device, 0,   ROT90,  "Century Electronics", "Space Warp?", MACHINE_IS_SKELETON )