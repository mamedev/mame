/*

    Pluto 5

    Skeleton Driver - For note keeping, no sets currently supported.

    68340 based system like MPU5/SC4
    used by JPM? Manufactuered by Heber Ltd.

    Known games
    Club DNA?


*/

#include "emu.h"
#include "cpu/m68000/m68000.h"

#if 0
class pluto5_state : public driver_device
{
public:
	pluto5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

};


static ADDRESS_MAP_START( pluto5_map, AS_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x2fffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START(  pluto5 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( pluto5, pluto5_state )
	MCFG_CPU_ADD("maincpu", M68020, 16000000)	 // 68340?
	MCFG_CPU_PROGRAM_MAP(pluto5_map)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	/* unknown sound */
MACHINE_CONFIG_END
#endif
