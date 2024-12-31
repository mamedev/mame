// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************************

    Genius KID ABC Fan
    Mis Primeras Lecciones
    Genius Junior Profi

    Other known undumped international versions:
    - Smart Start Elite (English version of Genius Junior Profi / Mis Primeras Lecciones)

    TODO: identify CPU type (16-bit processor internally, but with 8-bit external bus?)
    It might be that the dumped ROMs contain no actual code, only graphics data and
    sound samples.

***************************************************************************************/

#include "emu.h"


namespace {

class gkidabc_state : public driver_device
{
public:
	gkidabc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void gkidabc(machine_config &config);
};


static INPUT_PORTS_START(gkidabc)
INPUT_PORTS_END

void gkidabc_state::gkidabc(machine_config &config)
{
}


ROM_START(gkidabc)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("27-5730-00.bin", 0x00000, 0x20000, CRC(64664708) SHA1(74212c2dec1caa41dbc933b50f857904a8ac623b))
ROM_END

ROM_START(miprimlec)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("27-5482-01.u1", 0x00000, 0x20000, CRC(83aa655b) SHA1(5d7b03f0ff2836e228da77676df03854f87edd26))

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD( "csm10150an.u3", 0x0000, 0x2000, NO_DUMP ) // TSP50C10 (8K bytes of ROM) labeled "67ACLKT VIDEO TECH CSM10150AN"
ROM_END

ROM_START(gjrprofi)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("27-5476-00.u1", 0x00000, 0x20000, CRC(ad1ec838) SHA1(0cf90c02762ace656191a38ae423a4fa0e7484f7))
ROM_END

} // anonymous namespace


COMP(1996, gkidabc,   0, 0, gkidabc, gkidabc, gkidabc_state, empty_init, "VTech", "Genius KID ABC Fan (Germany)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
COMP(1995, miprimlec, 0, 0, gkidabc, gkidabc, gkidabc_state, empty_init, "VTech", "Mis Primeras Lecciones (Spain)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
COMP(1995, gjrprofi,  0, 0, gkidabc, gkidabc, gkidabc_state, empty_init, "VTech", "Genius Junior Profi (Germany)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
