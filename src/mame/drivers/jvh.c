#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/tms9900/tms9900.h"

class jvh_state : public driver_device
{
public:
	jvh_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};



static ADDRESS_MAP_START( jvh_map, AS_PROGRAM, 8, jvh_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( jvh )
INPUT_PORTS_END

void jvh_state::machine_reset()
{
}

static DRIVER_INIT( jvh )
{
}

static MACHINE_CONFIG_START( jvh, jvh_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS9980, 1000000)
	MCFG_CPU_PROGRAM_MAP(jvh_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Escape
/-------------------------------------------------------------------*/
ROM_START(escape)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_ic1.bin", 0x0000, 0x2000, CRC(fadb8f9a) SHA1(b7e7ea8e33847c14a3414f5e367e304f12c0bc00))
	ROM_LOAD("cpu_ic7.bin", 0x2000, 0x2000, CRC(2f9402b4) SHA1(3d3bae7e4e5ad40e3c8019d55392defdffd21cc4))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("snd.bin",     0xc000, 0x2000, CRC(2477bbe2) SHA1(f636952822153f43e9d09f8211edde1057249203))
	ROM_RELOAD(0xe000, 0x2000)
ROM_END

/*-------------------------------------------------------------------
/ Movie Masters
/-------------------------------------------------------------------*/
ROM_START(movmastr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mm_ic1.764", 0x0000, 0x2000, CRC(fb59920d) SHA1(05536c4c036a8d73516766e14f4449665b2ec180))
	ROM_LOAD("mm_ic7.764", 0x2000, 0x2000, CRC(9b47af41) SHA1(ae795c22aa437d6c71312d93de8a87f43ee500fb))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("snd.bin", 0xc000, 0x2000, NO_DUMP)
	ROM_RELOAD(0xe000, 0x2000)
ROM_END


GAME(1987,  escape,    0,  jvh,  jvh,  jvh,  ROT0,  "Jac Van Ham (Royal)",    "Escape",             GAME_IS_SKELETON_MECHANICAL)
GAME(19??,  movmastr,  0,  jvh,  jvh,  jvh,  ROT0,  "Jac Van Ham (Royal)",    "Movie Masters",      GAME_IS_SKELETON_MECHANICAL)
