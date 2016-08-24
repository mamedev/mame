// license:BSD-3-Clause
// copyright-holders:Ivan Vangelista

// Skeleton driver for IDSA pinballs.
// Known pinballs to be dumped: Fantastic Car (1986)
// Hardware listing and ROM definitions from PinMAME.

/*
	Hardware:
	---------
		CPU:     Z80 @ 4 MHz
			INT: IRQ @ 977 Hz (4MHz/2048/2) or 488 Hz (4MHz/2048/4)
		IO:      DMA, AY8910 ports
		DISPLAY: bsktball: 7-digit 7-segment panels with PROM-based 5-bit BCD data (allowing a simple alphabet)
		         v1: 6-digit 7-segment panels with BCD decoding
		SOUND:	 2 x AY8910 @ 2 MHz plus SP0256 @ 3.12 MHz on board
*/

#include "cpu/z80/z80.h"
#include "machine/genpin.h"
#include "sound/ay8910.h"
#include "sound/sp0256.h"

class idsa_state : public genpin_class
{
public:
	idsa_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu") { }

private:
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START( maincpu_map, AS_PROGRAM, 8, idsa_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( maincpu_io_map, AS_IO, 8, idsa_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

   	
static INPUT_PORTS_START( idsa )
INPUT_PORTS_END

static MACHINE_CONFIG_START( idsa, idsa_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(maincpu_map)
	MCFG_CPU_IO_MAP(maincpu_io_map)

	/* video hardware */
	//MCFG_DEFAULT_LAYOUT()

	/* sound hardware */
	//2 x ay8910
	//sp0256
	MCFG_FRAGMENT_ADD( genpin_audio )
MACHINE_CONFIG_END


ROM_START(v1)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("v1.128", 0x0000, 0x4000, CRC(4e08f7bc) SHA1(eb6ef00e489888dd9c53010c525840de06bcd0f3))

	ROM_REGION(0x10000, "sp0256", 0)
	ROM_LOAD("v1.128", 0x0000, 0x4000, CRC(4e08f7bc) SHA1(eb6ef00e489888dd9c53010c525840de06bcd0f3))
	ROM_RELOAD(0x4000, 0x4000)
	ROM_RELOAD(0x8000, 0x4000)
	ROM_RELOAD(0xc000, 0x4000)
ROM_END

ROM_START(bsktbllp)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("bsktball.256", 0x0000, 0x8000, CRC(d474e29b) SHA1(750cbacef34dde0b3dcb6c1e4679db78a73643fd))

	ROM_REGION(0x10000, "sp0256", 0)
	ROM_LOAD("bsktball.256", 0x0000, 0x8000, CRC(d474e29b) SHA1(750cbacef34dde0b3dcb6c1e4679db78a73643fd))
	ROM_RELOAD(0x8000, 0x8000)
ROM_END


GAME( 1985, v1, 0, idsa, idsa, driver_device, 0, ROT0, "IDSA", "V.1", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, bsktbllp, 0, idsa, idsa, driver_device, 0, ROT0, "IDSA", "Basket Ball", MACHINE_IS_SKELETON_MECHANICAL )
