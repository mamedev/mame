// license:BSD-3-Clause
// copyright-holders:David Haywood

/* Sansa Fuze */

// info can be found at
// http://www.rockbox.org/wiki/SansaFuze / http://www.rockbox.org/wiki/SansaAMSFirmware
// http://forums.rockbox.org/index.php?topic=14064
// http://daniel.haxx.se/sansa/ams.html

#include "emu.h"
#include "cpu/arm7/arm7.h"


namespace {

class sansa_fuze_state : public driver_device
{
public:
	sansa_fuze_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void sansa_fuze(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	void sansa_fuze_map(address_map &map) ATTR_COLD;
};



void sansa_fuze_state::sansa_fuze_map(address_map &map)
{
	map(0x00000000, 0x0001ffff).rom();

	map(0x80000000, 0x8001ffff).rom().region("maincpu", 0x00000);
	map(0x81000000, 0x81ffffff).ram();
}


static INPUT_PORTS_START( sansa_fuze )
INPUT_PORTS_END


void sansa_fuze_state::sansa_fuze(machine_config &config)
{
	/* basic machine hardware */
	ARM7(config, m_maincpu, 50000000); // arm based, speed unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &sansa_fuze_state::sansa_fuze_map);
}

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
	ROM_LOAD( "020333_clpp_data.dat",   0x00000, 0x566a0d, CRC(2093569c) SHA1(7882abcf000860a3071f5afe91719530bc54c68a) ) // original filename clpp_data.dat, actually a 6 minute MP3 advertisement for slotRadio

	ROM_REGION(0xf00000, "updates1", 0 )
	// 01.02.31 (for Fuze 1?)
	ROM_LOAD( "010231_fuzea.bin",   0x00000, 0xf00000, CRC(48b264cb) SHA1(387d5270fdd2fb7ba3901be59651a55167700768) ) // original filename fuzea.bin

ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT       CLASS             INIT        COMPANY    FULLNAME        FLAGS
CONS( 200?, sanfuze2, 0,      0,      sansa_fuze, sansa_fuze, sansa_fuze_state, empty_init, "Sandisk", "Sansa Fuze 2", MACHINE_IS_SKELETON )
