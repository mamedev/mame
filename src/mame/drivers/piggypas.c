// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Piggy Pass

main program rom contains the following near the start
COPYRIGHT (c) 1990, 1991, 1992, DOYLE & ASSOC., INC.   VERSION 04.40

hw platform unknown
game details unknown

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"

class piggypas_state : public driver_device
{
public:
	piggypas_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	virtual void machine_start();
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
};



static ADDRESS_MAP_START( piggypas_map, AS_PROGRAM, 8, piggypas_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( piggypas )
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



void piggypas_state::machine_start()
{
}

void piggypas_state::machine_reset()
{
}

static MACHINE_CONFIG_START( piggypas, piggypas_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,8000000) // wrong CPU? (not valid Z80 code)
	MCFG_CPU_PROGRAM_MAP(piggypas_map)
//	MCFG_CPU_VBLANK_INT_DRIVER("screen", piggypas_state,  irq0_line_hold)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH) // not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



ROM_START( piggypas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pigypass.u6", 0x00000, 0x10000, CRC(c8dc4e26) SHA1(f9643945f84fe2679742922abf5a92b77bf59e4c) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "pigypass.u14", 0x00000, 0x40000, CRC(855504c1) SHA1(dfe91943057fa66798c8395348cf703cb11468d2) )
ROM_END


GAME( 1992, piggypas,  0,    piggypas, piggypas, driver_device,  0, ROT0, "Doyle & Assoc.", "Piggy Pass (version 04.40)", MACHINE_IS_SKELETON_MECHANICAL )
