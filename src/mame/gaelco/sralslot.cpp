// license:BSD-3-Clause
// copyright-holders:

/* Skeleton driver for Gaelco / Covielsa "Super Rally" slot/gambling machine.
   The exact hardware configuration is unknown, but seems similar to Heber Pluto 1 hardware.
*/

#include "emu.h"

#include "machine/68340.h"

#include "speaker.h"

namespace {

class sralslot_state : public driver_device
{
public:
	sralslot_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void sralslot(machine_config &config) ATTR_COLD;
	void init_sral1990() ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void sralslot_map(address_map &map) ATTR_COLD;
};

void sralslot_state::sralslot_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x400000, 0x40ffff).ram();
}

static INPUT_PORTS_START( sralslot )
INPUT_PORTS_END

// Unknown hardware configuration. Everything is guessed from the ROMs contents.
void sralslot_state::sralslot(machine_config &config)
{
	M68340(config, m_maincpu, 16'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sralslot_state::sralslot_map);

	SPEAKER(config, "speaker", 2).front();
}

void sralslot_state::init_sral1990()
{
	uint16_t *src = &memregion("maincpu")->as_u16(); 
	int srcsize = memregion("maincpu")->bytes();
	// same basic scramble as pluto5.cpp, astrafr.cpp (also Heber platforms)
	std::vector<uint16_t> dst(srcsize / 2);

	for (int x = 0; x < srcsize / 2; x += 2)
	{
		dst[(x / 2)] = src[x];
		dst[(srcsize / 4) + (x / 2)] = src[x + 1];
	}

	memcpy(src, &dst[0], srcsize);
}


ROM_START( sralslot )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rally_0204_1_27c040.bin", 0x00000, 0x80000, CRC(e5fcdd99) SHA1(b0686580c26228ca4f3f7a8ace4090f63676d5d0) )
	ROM_LOAD16_BYTE( "rally_0204_2_27c040.bin", 0x00001, 0x80000, CRC(e3cd6bd8) SHA1(59ad84d382a8f14a3d3a6b60f692315f8713560a) )
ROM_END

ROM_START( sralslota )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "super_rally_esp_rveall_0203.1_27c4001.bin", 0x00000, 0x80000, CRC(ea290da9) SHA1(bf572fbbaf984fe6bed6ab0d16624810bc265389) )
	ROM_LOAD16_BYTE( "super_rally_esp_rveall_0203.2_27c4001.bin", 0x00001, 0x80000, CRC(3509b59d) SHA1(423f99e1a73149a20b0aca4e542649ecfbd19000) )
ROM_END

ROM_START( sralslotb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rall_0201.1_27c4001.bin", 0x00000, 0x80000, CRC(8bcaed5b) SHA1(6cbfbc9554e668f88dcf8dc4f216a2e4f711617c) )
	ROM_LOAD16_BYTE( "rall_0201.2_27c040.bin",  0x00001, 0x80000, CRC(bdaf87eb) SHA1(8f0f58afa21efbe6c7fa979bc302277c912cf60c) )
ROM_END

ROM_START( sralslotc )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rally_spanje_rverall_0107.1_27c4001.bin", 0x00000, 0x80000, CRC(dfbd4e69) SHA1(8aebcb226dc391adc5394e7847155d5aac69e76e) )
	ROM_LOAD16_BYTE( "rally_spanje_rverall_0107.2.bin",         0x00001, 0x80000, NO_DUMP )
ROM_END

} // anonymous namespace

/* Spanish legal registry suggests a 1990 date, but the revisions here at least are from 95/96

   Either this is misidentified (another 'Super Rally' game?) or this is a rebuild on different hardware
   as if we assume this is running on a Heber Pluto platform (Pluto 1?) it can't have been earlier than 94
   
   1345    3AB-1986    SUPER RALLY    01.01.1990    1EC-00000016    COVIELSA, S.A
*/

GAME( 1996, sralslot,  0,        sralslot, sralslot, sralslot_state, init_sral1990, ROT0, "Gaelco / Covielsa", "Super Rally (v0204, 2-Feb-1996)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1995, sralslota, sralslot, sralslot, sralslot, sralslot_state, init_sral1990, ROT0, "Gaelco / Covielsa", "Super Rally (v0203, 19-Dec-1995)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1995, sralslotb, sralslot, sralslot, sralslot, sralslot_state, init_sral1990, ROT0, "Gaelco / Covielsa", "Super Rally (v0201, 6-Nov-1995)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1995, sralslotc, sralslot, sralslot, sralslot, sralslot_state, init_sral1990, ROT0, "Gaelco / Covielsa", "Super Rally (v0107)",              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
