#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/tms9900/tms9900.h"

class nsm_state : public driver_device
{
public:
	nsm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( nsm_map, AS_PROGRAM, 8, nsm_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( nsm )
INPUT_PORTS_END

void nsm_state::machine_reset()
{
}

static DRIVER_INIT( nsm )
{
}

static MACHINE_CONFIG_START( nsm, nsm_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS9995, 11052000)
	MCFG_CPU_PROGRAM_MAP(nsm_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Cosmic Flash (1985)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Hot Fire Birds (1985)
/-------------------------------------------------------------------*/
ROM_START(firebird)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("nsmf02.764", 0x0000, 0x2000, CRC(236b5780) SHA1(19ef6e1fc900e5d94f615a4316f0383ed5ee939c))
	ROM_LOAD("nsmf03.764", 0x2000, 0x2000, CRC(d88c6ef5) SHA1(00edeefaab7e1141741aa132e6f7e56a911573be))
	ROM_LOAD("nsmf04.764", 0x4000, 0x2000, CRC(38a8add4) SHA1(74f781edc31aad07411feacad53c5f6cc73d09f4))
ROM_END

/*-------------------------------------------------------------------
/ Tag-Team Pinball (1986)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ The Games (1985)
/-------------------------------------------------------------------*/

GAME(1985,  firebird,  0,  nsm,  nsm,  nsm,  ROT0,  "NSM",    "Hot Fire Birds",     GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
