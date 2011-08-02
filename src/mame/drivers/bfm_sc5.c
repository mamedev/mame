/*

	Scorpion 5

    Skeleton Driver - For note keeping, no sets currently supported pending a better
	 understanding of the system.

	Several sets have large roms, containing strings which mention Compact Flash cards
	 - Are these CF card dumps
	 - Should *All* games have them?
	 - Do the Program roms just override parts of this, or do they work together?


*/

#include "emu.h"
#include "cpu/m68000/m68000.h"

#if 0
class sc5_state : public driver_device
{
public:
	sc5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

};


static ADDRESS_MAP_START( sc5_map, AS_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x2fffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START(  sc5 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( sc5, sc5_state )
	MCFG_CPU_ADD("maincpu", M68020, 16000000)	 // 68340?
	MCFG_CPU_PROGRAM_MAP(sc5_map)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	/* unknown sound */
MACHINE_CONFIG_END
#endif
