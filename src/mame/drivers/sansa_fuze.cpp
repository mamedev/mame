/* Sansa Fuze */

// info can be found at
// http://www.rockbox.org/wiki/SansaFuze / http://www.rockbox.org/wiki/SansaAMSFirmware
// http://forums.rockbox.org/index.php?topic=14064
// http://daniel.haxx.se/sansa/ams.html

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"


class sansa_fuze_state : public driver_device
{
public:
	sansa_fuze_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;
};



static ADDRESS_MAP_START( sansa_fuze_map, AS_PROGRAM, 32, sansa_fuze_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM

	AM_RANGE(0x80000000, 0x8001ffff) AM_ROM  AM_REGION("maincpu", 0x00000)
	AM_RANGE(0x81000000, 0x81ffffff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( sansa_fuze )
INPUT_PORTS_END


static MACHINE_CONFIG_START( sansa_fuze, sansa_fuze_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM7, 50000000) // arm based, speed unknown
	MCFG_CPU_PROGRAM_MAP(sansa_fuze_map)

MACHINE_CONFIG_END

ROM_START( sanfuze2 )
	ROM_REGION(0x20000, "maincpu", 0 )
	// this rom was dumped using the RockBox (custom firmware) debugging features, it's presumably the internal ROM
	// I don't know if it's specific to the Fuze 2, or shared.
	ROM_LOAD( "sanza.rom",   0x00000, 0x20000, CRC(a93674b0) SHA1(3a17dfc9ad31f07fdd66e3dac94c9494c59f3203) )
	
	// these are firmware update files, we probably need to extract the data from them in order to use them with the ROM
	// above and get things booting.
	// http://forums.sandisk.com/t5/Sansa-Fuze/Sansa-Fuze-Firmware-Update-01-02-31-amp-02-03-33/td-p/139175

	ROM_REGION(0xf00000, "updates2", 0 )
	/// 02.03.33  (for Fuze 2?)
	ROM_LOAD( "020333_fuzpa.bin",   0x00000, 0xf00000, CRC(045ec5be) SHA1(d951d93ff1c0a50343e0cf8e6997930b7c94e5ad) ) // original filename fuzpa.bin
	ROM_LOAD( "020333_clpp_data.dat",   0x00000, 0x566a0d, CRC(2093569c) SHA1(7882abcf000860a3071f5afe91719530bc54c68a) ) // original filename clpp_data.dat

	ROM_REGION(0xf00000, "updates1", 0 )
    // 01.02.31 (for Fuze 1?)
	ROM_LOAD( "010231_fuzea.bin",   0x00000, 0xf00000, CRC(48b264cb) SHA1(387d5270fdd2fb7ba3901be59651a55167700768) ) // original filename fuzea.bin

ROM_END


GAME( 200?, sanfuze2, 0,        sansa_fuze,  sansa_fuze, driver_device,  0,  ROT0, "Sandisk", "Sansa Fuze 2", MACHINE_NO_SOUND | MACHINE_IS_SKELETON )
