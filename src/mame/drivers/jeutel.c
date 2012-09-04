/*******************************************************************************

Pinball
Jeutel

********************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"

class jeutel_state : public driver_device
{
public:
	jeutel_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
public:
	DECLARE_DRIVER_INIT(jeutel);
};


static ADDRESS_MAP_START( jeutel_map, AS_PROGRAM, 8, jeutel_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	//AM_RANGE(0xe000, 0xe003) AM_READWRITE ppi8255_2
ADDRESS_MAP_END

static ADDRESS_MAP_START( jeutel_cpu2, AS_PROGRAM, 8, jeutel_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	//AM_RANGE(0x2000, 0x2003) AM_WRITE ppi8255_0
	//AM_RANGE(0x3000, 0x3003) AM_WRITE ppi8255_1
	//AM_RANGE(0x4000, 0x4000) AM_WRITENOP
	AM_RANGE(0x8000, 0x83ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( jeutel_cpu3, AS_PROGRAM, 8, jeutel_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( jeutel_cpu3_io, AS_IO, 8, jeutel_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x00, 0x00) AM_WRITE AY8910_control_port_0_w
	//AM_RANGE(0x01, 0x01) AM_WRITE AY8910_write_port_0_w
	//AM_RANGE(0x04, 0x04) AM_READ(AY8910_read_port_0_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( jeutel )
INPUT_PORTS_END

void jeutel_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(jeutel_state,jeutel)
{
}

static MACHINE_CONFIG_START( jeutel, jeutel_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 3300000)
	MCFG_CPU_PROGRAM_MAP(jeutel_map)
	MCFG_CPU_ADD("cpu2", Z80, 3300000)
	MCFG_CPU_PROGRAM_MAP(jeutel_cpu2)
	MCFG_CPU_ADD("cpu3", Z80, 3300000)
	MCFG_CPU_PROGRAM_MAP(jeutel_cpu3)
	MCFG_CPU_IO_MAP(jeutel_cpu3_io)
MACHINE_CONFIG_END

/*--------------------------------
/ Le King
/-------------------------------*/
ROM_START(leking)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("game-m.bin", 0x0000, 0x2000, CRC(4b66517a) SHA1(1939ea78932d469a16441507bb90b032c5f77b1e))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("game-v.bin", 0x0000, 0x1000, CRC(cbbc8b55) SHA1(4fe150fa3b565e5618896c0af9d51713b381ed88))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("sound-v.bin", 0x0000, 0x1000, CRC(36130e7b) SHA1(d9b66d43b55272579b3972005355b8a18ce6b4a9))
	ROM_LOAD("sound-p.bin", 0x1000, 0x2000, BAD_DUMP CRC(97eedd6c) SHA1(3bb8e5d32417c49ef97cbe407f2c5eeb214bf72d))
ROM_END

/*--------------------------------
/ Olympic Games
/-------------------------------*/
ROM_START(olympic)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("game-jo1.bin", 0x0000, 0x2000, CRC(c9f040cf) SHA1(c689f3a82d904d3f9fc8688d4c06082c51645b2f))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("game-v.bin", 0x0000, 0x1000, CRC(cd284a20) SHA1(94568e1247994c802266f9fbe4a6f6ed2b55a978))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("sound-j0.bin", 0x0000, 0x1000, CRC(5c70ce72) SHA1(b0b6cc7b6ec3ed9944d738b61a0d144b77b07000))
	ROM_LOAD("sound-p.bin", 0x1000, 0x2000, CRC(97eedd6c) SHA1(3bb8e5d32417c49ef97cbe407f2c5eeb214bf72d))
ROM_END


GAME(1983,  leking,   0,  jeutel,  jeutel, jeutel_state,  jeutel,  ROT0, "Jeutel", "Le King", GAME_IS_SKELETON_MECHANICAL)
GAME(1984,  olympic,  0,  jeutel,  jeutel, jeutel_state,  jeutel,  ROT0, "Jeutel", "Olympic Games", GAME_IS_SKELETON_MECHANICAL)
