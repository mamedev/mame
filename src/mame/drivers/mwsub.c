// license:BSD-3-Clause
// copyright-holders:hap
/* Midway's Submarine hardware, game number 760


*/

#include "emu.h"
#include "cpu/z80/z80.h"

#include "submar.lh"


class submar_state : public driver_device
{
public:
	submar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;
};






static ADDRESS_MAP_START( submar_map, AS_PROGRAM, 8, submar_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x207f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( submar_portmap, AS_IO, 8, submar_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END



static INPUT_PORTS_START( submar )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "01" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "02" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "04" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "08" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END




static MACHINE_CONFIG_START( submar, submar_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_19_968MHz/8)
	MCFG_CPU_PERIODIC_INT_DRIVER(submar_state, irq0_line_hold, 960.516) // 555 IC - where is irqack?
	MCFG_CPU_PROGRAM_MAP(submar_map)
	MCFG_CPU_IO_MAP(submar_portmap)

	/* no video! */

	/* sound hardware */
	//...
MACHINE_CONFIG_END


ROM_START( submar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sub.a1", 0x0000, 0x0800, CRC(bcef5db4) SHA1(8ae5099672fbdb7bcdc617e1f8cbc5435fbb738a) )
	ROM_LOAD( "sub.a2", 0x0800, 0x0800, CRC(f5780dd0) SHA1(f775dd6f64a730a2fb6c9baf5787698434150bc5) )
ROM_END



GAMEL( 1979, submar, 0, submar, submar, driver_device, 0, ROT0, "Midway", "Submarine (Midway)", GAME_NO_SOUND | GAME_NOT_WORKING | GAME_MECHANICAL, layout_submar )
