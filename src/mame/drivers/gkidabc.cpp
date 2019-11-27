// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************************

    Genius KID ABC Fan
    Mis Primeras Lecciones

    Other known undumped international versions:
    - Genius Junior Profi (German version of Mis Primeras Lecciones)
    - Smart Start Elite (English version of Genius Junior Profi)

    TODO: identify CPU type (16-bit processor internally, but with 8-bit external bus?)
    It might be that the dumped ROMs contain no actual code, only graphics data and
    sound samples.

***************************************************************************************/

#include "emu.h"

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
ROM_END


COMP(1996, gkidabc,   0, 0, gkidabc, gkidabc, gkidabc_state, empty_init, "VTech", "Genius KID ABC Fan (Germany)",   MACHINE_IS_SKELETON)
COMP(1995, miprimlec, 0, 0, gkidabc, gkidabc, gkidabc_state, empty_init, "VTech", "Mis Primeras Lecciones (Spain)", MACHINE_IS_SKELETON)
