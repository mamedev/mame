/***************************************************************************

    Skeleton driver for Hanimex Pencil II
    Manufactured by Soundic, Hong Kong.

    2012-11-06

    TODO:
    - Everything - this is just a skeleton


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/tms9928a.h"
//#include "imagedev/cartslot.h"
//#include "imagedev/cassette.h"


class pencil2_state : public driver_device
{
public:
	pencil2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;
	DECLARE_READ8_MEMBER(port00_r) { return 0x80; };
	DECLARE_READ8_MEMBER(port0f_r) { return 0x05; };
	virtual void machine_reset();
};

static ADDRESS_MAP_START(pencil2_mem, AS_PROGRAM, 8, pencil2_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_MIRROR(0x1800) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(pencil2_io, AS_IO, 8, pencil2_state)
	//ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(port00_r)
	AM_RANGE(0x0f, 0x0f) AM_READ(port0f_r)
	AM_RANGE(0x20, 0x20) AM_WRITENOP
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( pencil2 )
INPUT_PORTS_END


void pencil2_state::machine_reset()
{
}

static TMS9928A_INTERFACE(pencil2_tms9928a_interface)
{
	"screen",
	0x4000,
	DEVCB_NULL
};

static MACHINE_CONFIG_START( pencil2, pencil2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 3580000)
	MCFG_CPU_PROGRAM_MAP(pencil2_mem)
	MCFG_CPU_IO_MAP(pencil2_io)

	/* video hardware */
	MCFG_TMS9928A_ADD( "tms9928a", TMS9928A, pencil2_tms9928a_interface )
	MCFG_TMS9928A_SCREEN_ADD_PAL( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pencil2 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "mt.u4", 0x0000, 0x2000, CRC(338d7b59) SHA1(2f89985ac06971e00210ff992bf1e30a296d10e7) )
	ROM_LOAD( "1-or",  0xa000, 0x1000, CRC(1ddedccd) SHA1(5fc0d30b5997224b67bf286725468194359ced5a) )
	ROM_LOAD( "203",   0x8000, 0x2000, CRC(f502175c) SHA1(cb2190e633e98586758008577265a7a2bc088233) )
	ROM_LOAD( "202",   0xc000, 0x2000, CRC(5171097d) SHA1(171999bc04dc98c74c0722b2866310d193dc0f82) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT     STATE         INIT  COMPANY    FULLNAME       FLAGS */
COMP( 19??, pencil2,   0,     0,     pencil2,   pencil2, driver_device,  0,  "Hanimex", "Pencil II", GAME_IS_SKELETON)
