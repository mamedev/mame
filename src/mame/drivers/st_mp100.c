/*
    Stern MP-100 MPU
    (almost identical to Bally MPU-17)
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"

class st_mp100_state : public driver_device
{
public:
	st_mp100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( st_mp100_map, AS_PROGRAM, 8, st_mp100_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( st_mp100 )
INPUT_PORTS_END

void st_mp100_state::machine_reset()
{
}

static DRIVER_INIT( st_mp100 )
{
}

static MACHINE_CONFIG_START( st_mp100, st_mp100_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(st_mp100_map)
MACHINE_CONFIG_END

/*-------------------------------------
/ Cosmic Princess - same ROMs as Magic
/-------------------------------------*/
ROM_START(princess)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(8838091f) SHA1(d2702b5e15076793b4560c77b78eed6c1da571b6))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(fb955a6f) SHA1(387080d5af318463475797fecff026d6db776a0c))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Dracula
/-------------------------------*/
ROM_START(dracula)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(063f8b5e) SHA1(80434de549102bff829b474603d6736b839b8999))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(00336fbc) SHA1(d2c360b8a80b209ecf4ec02ee19a5234c0364504))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Hot Hand - uses MPU-200 inports
/-------------------------------*/
ROM_START(hothand)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(5e79ea2e) SHA1(9b45c59b2076fcb3a35de1dd3ba2444ea852f149))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(fb955a6f) SHA1(387080d5af318463475797fecff026d6db776a0c))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Lectronamo
/-------------------------------*/
ROM_START(lectrono)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(79e918ff) SHA1(a728eb26d941a9c7484be593a216905237d32551))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(7c6e5fb5) SHA1(3aa4e0c1f377ba024e6b34bd431a188ff02d4eaa))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Magic - uses MPU-200 inports
/-------------------------------*/
ROM_START(magic)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(8838091f) SHA1(d2702b5e15076793b4560c77b78eed6c1da571b6))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(fb955a6f) SHA1(387080d5af318463475797fecff026d6db776a0c))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Memory Lane
/-------------------------------*/
ROM_START(memlane)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(aff1859d) SHA1(5a9801d139bf2477b6d351a2654ae07516be144a))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(3e236e3c) SHA1(7f631a5fac8a1b1af3b5332ba38d52553f13531a))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Nugent
/-------------------------------*/
ROM_START(nugent)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(79e918ff) SHA1(a728eb26d941a9c7484be593a216905237d32551))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(7c6e5fb5) SHA1(3aa4e0c1f377ba024e6b34bd431a188ff02d4eaa))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Pinball
/-------------------------------*/
ROM_START(pinball)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(1db32a33) SHA1(2f0a3ca36968b81f29373e4f2cf7ee28a4071882))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(432e9b9e) SHA1(292e509f50bc841f6e469c198fc82c2a9095f008))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Stars
/-------------------------------*/
ROM_START(stars)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(630d05df) SHA1(2baa16265d524297332fa951d9eab3e0e8d26078))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(57e63d42) SHA1(619ef955553654893c3071d8b70855fee8a5e6a7))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Stingray
/-------------------------------*/
ROM_START(stingray)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(1db32a33) SHA1(2f0a3ca36968b81f29373e4f2cf7ee28a4071882))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(432e9b9e) SHA1(292e509f50bc841f6e469c198fc82c2a9095f008))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Trident - uses MPU-200 inports
/-------------------------------*/
ROM_START(trident)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(934e49dd) SHA1(cbf6ca2759166f522f651825da0c75cf7248d3da))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(540bce56) SHA1(0b21385501b83e448403e0216371487ed54026b7))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Wildfyre
/-------------------------------*/
ROM_START(wildfyre)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(063f8b5e) SHA1(80434de549102bff829b474603d6736b839b8999))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(00336fbc) SHA1(d2c360b8a80b209ecf4ec02ee19a5234c0364504))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

GAME(1979,	princess,	0,		st_mp100,	st_mp100,	st_mp100,	ROT0,	"Stern",	"Cosmic Princess",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1979,	dracula,	0,		st_mp100,	st_mp100,	st_mp100,	ROT0,	"Stern",	"Dracula",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1979,	hothand,	0,		st_mp100,	st_mp100,	st_mp100,	ROT0,	"Stern",	"Hot Hand",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	lectrono,	0,		st_mp100,	st_mp100,	st_mp100,	ROT0,	"Stern",	"Lectronamo",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1979,	magic,		0,		st_mp100,	st_mp100,	st_mp100,	ROT0,	"Stern",	"Magic",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	memlane,	0,		st_mp100,	st_mp100,	st_mp100,	ROT0,	"Stern",	"Memory Lane",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	nugent,		0,		st_mp100,	st_mp100,	st_mp100,	ROT0,	"Stern",	"Nugent",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1977,	pinball,	0,		st_mp100,	st_mp100,	st_mp100,	ROT0,	"Stern",	"Pinball",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	stars,		0,		st_mp100,	st_mp100,	st_mp100,	ROT0,	"Stern",	"Stars",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1977,	stingray,	0,		st_mp100,	st_mp100,	st_mp100,	ROT0,	"Stern",	"Stingray",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1979,	trident,	0,		st_mp100,	st_mp100,	st_mp100,	ROT0,	"Stern",	"Trident",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	wildfyre,	0,		st_mp100,	st_mp100,	st_mp100,	ROT0,	"Stern",	"Wildfyre",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
