#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"

class ltd_state : public driver_device
{
public:
	ltd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( ltd_map, AS_PROGRAM, 8, ltd_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( ltd )
INPUT_PORTS_END

void ltd_state::machine_reset()
{
}

static DRIVER_INIT( ltd )
{
}

static MACHINE_CONFIG_START( ltd, ltd_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6803, 1000000)
	MCFG_CPU_PROGRAM_MAP(ltd_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Al Capone
/-------------------------------------------------------------------*/
ROM_START(alcapone)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("alcapo_l.bin", 0xc000, 0x1000, CRC(c4270ba8) SHA1(f3d80af9900c94df2d43f2755341a346a0b64c87))
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("alcapo_h.bin", 0xd000, 0x1000, CRC(279f766d) SHA1(453c58e44c4ef8f1f9eb752b6163c61ebed70b27))
	ROM_RELOAD(0xf000, 0x1000)
ROM_END

/*-------------------------------------------------------------------
/ Atlantis
/-------------------------------------------------------------------*/
ROM_START(atla_ltd)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("atlantis.bin", 0xc000, 0x0800, CRC(c61be043) SHA1(e6c4463f59a5743fa34aa55beeb6f536ad9f1b56))
	ROM_RELOAD(0xc800, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_RELOAD(0xd800, 0x0800)
	ROM_RELOAD(0xe000, 0x0800)
	ROM_RELOAD(0xe800, 0x0800)
	ROM_RELOAD(0xf000, 0x0800)
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Black Hole
/-------------------------------------------------------------------*/
ROM_START(bhol_ltd)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("blackhol.bin", 0xc000, 0x0800, CRC(9f6ae35e) SHA1(c17bf08a41c6cf93550671b0724c58e8ac302c33))
	ROM_RELOAD(0xc800, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_RELOAD(0xd800, 0x0800)
	ROM_RELOAD(0xe000, 0x0800)
	ROM_RELOAD(0xe800, 0x0800)
	ROM_RELOAD(0xf000, 0x0800)
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Columbia
/-------------------------------------------------------------------*/
ROM_START(columbia)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("columb-d.bin", 0xc000, 0x1000, NO_DUMP)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("columb-e.bin", 0xd000, 0x1000, CRC(013abca0) SHA1(269376af92368d214c3d09ec6d3eb653841666f3))
	ROM_RELOAD(0xf000, 0x1000)
ROM_END

/*-------------------------------------------------------------------
/ Cowboy Eight Ball
/-------------------------------------------------------------------*/
ROM_START(cowboy)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cowboy_l.bin", 0xc000, 0x1000, CRC(87befe2a) SHA1(93fdf40b10e53d7d95e5dc72923b6be887411fc0))
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("cowboy_h.bin", 0xd000, 0x1000, CRC(105e5d7b) SHA1(75edeab8c8ba19f334479133802acbc25f405763))
	ROM_RELOAD(0xf000, 0x1000)
ROM_END

/*-------------------------------------------------------------------
/ Mr. & Mrs. Pec-Men
/-------------------------------------------------------------------*/
ROM_START(pecmen)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("pecmen_l.bin", 0xc000, 0x1000, CRC(f86c724e) SHA1(635ec94a1c6e77800ef9774102cc639be86c4261))
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("pecmen_h.bin", 0xd000, 0x1000, CRC(013abca0) SHA1(269376af92368d214c3d09ec6d3eb653841666f3))
	ROM_RELOAD(0xf000, 0x1000)
ROM_END

/*-------------------------------------------------------------------
/ Zephy
/-------------------------------------------------------------------*/
ROM_START(zephy)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("zephy.l2", 0xc000, 0x1000, CRC(8dd11287) SHA1(8133d0c797eb0fdb56d83fc55da91bfc3cddc9e3))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
ROM_END


GAME(198?,	alcapone,	0,		ltd,	ltd,	ltd,	ROT0,	"LTD",	"Al Capone",			GAME_IS_SKELETON_MECHANICAL)
GAME(19??,	atla_ltd,	0,		ltd,	ltd,	ltd,	ROT0,	"LTD",	"Atlantis (LTD)",		GAME_IS_SKELETON_MECHANICAL)
GAME(19??,	bhol_ltd,	0,		ltd,	ltd,	ltd,	ROT0,	"LTD",	"Black Hole (LTD)",		GAME_IS_SKELETON_MECHANICAL)
GAME(198?,	columbia,	0,		ltd,	ltd,	ltd,	ROT0,	"LTD",	"Columbia",				GAME_IS_SKELETON_MECHANICAL)
GAME(198?,	cowboy,		0,		ltd,	ltd,	ltd,	ROT0,	"LTD",	"Cowboy Eight Ball",	GAME_IS_SKELETON_MECHANICAL)
GAME(198?,	pecmen,		0,		ltd,	ltd,	ltd,	ROT0,	"LTD",	"Mr. & Mrs. Pec-Men",	GAME_IS_SKELETON_MECHANICAL)
GAME(198?,	zephy,		0,		ltd,	ltd,	ltd,	ROT0,	"LTD",	"Zephy",				GAME_IS_SKELETON_MECHANICAL)
