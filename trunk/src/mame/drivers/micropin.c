#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"

class micropin_state : public driver_device
{
public:
	micropin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( micropin_map, AS_PROGRAM, 8, micropin_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( micropin )
INPUT_PORTS_END

void micropin_state::machine_reset()
{
}

static DRIVER_INIT( micropin )
{
}

static MACHINE_CONFIG_START( micropin, micropin_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(micropin_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Pentacup
/-------------------------------------------------------------------*/
ROM_START(pentacup)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic2.bin", 0x6400, 0x0400, CRC(fa468a0f) SHA1(e9c8028bcd5b87d24f4588516536767a869c38ff))
	ROM_LOAD("ic3.bin", 0x6800, 0x0400, CRC(7bfdaec8) SHA1(f2037c0e2d4acf0477351ecafc9f0826e9d64d76))
	ROM_LOAD("ic4.bin", 0x6c00, 0x0400, CRC(5e0fcb1f) SHA1(e529539c6eb1e174a799ad6abfce9e31870ff8af))
	ROM_LOAD("ic5.bin", 0x7000, 0x0400, CRC(a26c6e0b) SHA1(21c4c306fbc2da52887e309b1c83a1ea69501c1f))
	ROM_LOAD("ic6.bin", 0x7400, 0x0400, CRC(4715ac34) SHA1(b6d8c20c487db8d7275e36f5793666cc591a6691))
	ROM_LOAD("ic7.bin", 0x7800, 0x0400, CRC(c58d13c0) SHA1(014958bc69ff326392a5a7782703af0980e6e170))
	ROM_LOAD("ic8.bin", 0x7c00, 0x0400, CRC(9f67bc65) SHA1(504008d4c7c23a14fdf247c9e6fc00e95d907d7b))
	ROM_RELOAD(0xfc00, 0x0400)
ROM_END

ROM_START(pentacup2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("micro_1.bin", 0x0000, 0x0800, CRC(4d6dc218) SHA1(745c553f3a42124f925ca8f2e52fd08d05999594))
	ROM_LOAD("micro_2.bin", 0x0800, 0x0800, CRC(33cd226d) SHA1(d1dff8445a0f35da09d560a16038c969845ff21f))
	ROM_LOAD("micro_3.bin", 0x1000, 0x0800, CRC(997bde74) SHA1(c3ea33f7afbdc7f2a22798a13ec323d7c6628dd4))
	ROM_LOAD("micro_4.bin", 0x1800, 0x0800, CRC(a804e7d6) SHA1(f414d6a5308266744645849940c00cd422e920d2))
ROM_END


GAME(1978,  pentacup,  0,         micropin,  micropin,  micropin,  ROT0,  "Micropin",    "Pentacup (rev. 1)",     GAME_IS_SKELETON_MECHANICAL)
GAME(1980,  pentacup2, pentacup,  micropin,  micropin,  micropin,  ROT0,  "Micropin",    "Pentacup (rev. 2)",     GAME_IS_SKELETON_MECHANICAL)
