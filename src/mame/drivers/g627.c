/*
    Pinball
    Midway A084-91313-G627
           A080-91313-G627
           A082-91320-C000

    Only one of its kind
*/


#include "emu.h"
#include "cpu/z80/z80.h"

class g627_state : public driver_device
{
public:
	g627_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
public:
	DECLARE_DRIVER_INIT(g627);
};


static ADDRESS_MAP_START( g627_map, AS_PROGRAM, 8, g627_state )
	AM_RANGE(0x0000, 0x17ff) AM_ROM
	AM_RANGE(0xc000, 0xc0ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( g627_io, AS_IO, 8, g627_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x00, 0x02) AM_WRITE(disp_w)
	//AM_RANGE(0x03, 0x07) AM_WRITE(port_0x_w)
	//AM_RANGE(0x10, 0x17) AM_WRITE(port_1x_w)
	//AM_RANGE(0x21, 0x22) AM_READ(port_2x_r)
	//AM_RANGE(0x20, 0x25) AM_WRITE(port_2x_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( g627 )
INPUT_PORTS_END

void g627_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(g627_state,g627)
{
}

static MACHINE_CONFIG_START( g627, g627_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 14138000/8)
	MCFG_CPU_PROGRAM_MAP(g627_map)
	MCFG_CPU_IO_MAP(g627_io)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Rotation VIII (09/1978)
/-------------------------------------------------------------------*/
ROM_START(rotation)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rot-a117.dat", 0x0000, 0x0800, CRC(7bb6beb3) SHA1(5ee62246032158c68d426c11a4a9a889ee7655d7))
	ROM_LOAD("rot-b117.dat", 0x0800, 0x0800, CRC(538e37b2) SHA1(d283ac4d0024388b92b6494fcde63957b705bf48))
	ROM_LOAD("rot-c117.dat", 0x1000, 0x0800, CRC(3321ff08) SHA1(d6d94fea27ef58ca648b2829b32d62fcec108c9b))
ROM_END


GAME(1978,  rotation,  0,  g627,  g627, g627_state,  g627,  ROT0,  "Midway",    "Rotation VIII",      GAME_IS_SKELETON_MECHANICAL)
