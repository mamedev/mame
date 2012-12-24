/***************************************************************************

    Skeleton driver for Hanimex Pencil II
    Manufactured by Soundic, Hong Kong.

    2012-11-06

    TODO:
    - Everything - this is just a skeleton


    Information found by looking inside the computer
    ------------------------------------------------
Main Board PEN-002 11-50332-10

J1 Expansion slot
J2 Cart slot
J3 Memory expansion slot
J4 Printer slot
J5,J6 Joystick ports

XTAL 10.738 MHz

Output is to a TV on Australian Channel 1 (no monitor output)

U1     uPD780C-1 (Z80A)
U2     Video chip with heatsink stuck on top, possibly TMS9928
U3     SN76489AN
U4     2764 bios rom
U5     uPD4016C-2 (assumed to be equivalent of 6116 2K x 8bit static RAM)
U6     74LS04
U7     74LS74A
U8-10  74LS138
U11    74LS00
U12    74LS273
U13    74LS74A
U14-21 TMM416P-3 (4116-3 16k x 1bit dynamic RAM)
U22    74LS05
U23-24 SN74LS541

BASIC CART PEN-700 11-50332-31 Rev.0
SD-BASIC VERSION 2.0 FOR PENCIL II
(c) 1983 SOUNDIC ELECTRONICS LTD HONG KONG ALL RIGHTS RESERVED
1 x 2732
2 x 2764
The roms were dumped by attaching a cable from the printer port to
a Super-80 and writing programs in Basic to transfer the bytes.
Therefore it is not known which rom "202" or "203" is which address range.


MEMORY MAP
0000-1FFF bios rom
2000-5FFF available for expansion
6000-7FFF static RAM (2K mirrored)
8000-FFFF cart slot

The 16k dynamic RAM holds the BASIC program and the video/gfx etc
but is banked out of view of a BASIC program.


KNOWN CARTS
SD-BASIC V1.0
SD-BASIC V2.0


ToDo:
- Keyboard
- Printer (out 00)
- Cassette
- Joysticks
- Colours are different to the real system
- RAM banking
- Cart slot
- Size of vram
- When BASIC starts the intro is off the top of the screen

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/tms9928a.h"
#include "sound/sn76496.h"
//#include "imagedev/cartslot.h"
//#include "imagedev/cassette.h"
//#include "sound/wave.h"


class pencil2_state : public driver_device
{
public:
	pencil2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;
	//DECLARE_READ8_MEMBER(port00_r) { return 0x80; };
	//DECLARE_READ8_MEMBER(port0f_r) { return 0x05; };
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
	//AM_RANGE(0x00, 0x00) AM_READ(port00_r)
	//AM_RANGE(0x0f, 0x0f) AM_READ(port0f_r)
	//AM_RANGE(0x20, 0x20) AM_WRITENOP
	AM_RANGE(0xbe, 0xbe) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0xbf, 0xbf) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)
	AM_RANGE(0xe0, 0xff) AM_DEVWRITE("sn76489a", sn76489a_device, write)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( pencil2 )
INPUT_PORTS_END


void pencil2_state::machine_reset()
{
}

static const sn76496_config psg_intf =
{
	DEVCB_NULL
};

static TMS9928A_INTERFACE(pencil2_tms9928a_interface)
{
	"screen",	// screen tag
	0x4000,		// vram size (guess)
	DEVCB_NULL	// write line if int changes
};

static MACHINE_CONFIG_START( pencil2, pencil2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_10_738635MHz/3)
	MCFG_CPU_PROGRAM_MAP(pencil2_mem)
	MCFG_CPU_IO_MAP(pencil2_io)

	/* video hardware */
	MCFG_TMS9928A_ADD( "tms9928a", TMS9928A, pencil2_tms9928a_interface )
	MCFG_TMS9928A_SCREEN_ADD_PAL( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sn76489a", SN76489A, XTAL_10_738635MHz/3) // guess
	MCFG_SOUND_CONFIG(psg_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
//	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
//	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* cassette */
//	MCFG_CASSETTE_ADD( CASSETTE_TAG, pencil2_cassette_interface )

	/* cartridge */
//	MCFG_CARTSLOT_ADD("cart")
//	MCFG_CARTSLOT_EXTENSION_LIST("rom")
//	MCFG_CARTSLOT_NOT_MANDATORY
//	MCFG_CARTSLOT_LOAD(pencil2_cart)
//	MCFG_CARTSLOT_INTERFACE("pencil2_cart")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pencil2 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "mt.u4", 0x0000, 0x2000, CRC(338d7b59) SHA1(2f89985ac06971e00210ff992bf1e30a296d10e7) )
	ROM_LOAD( "1-or",  0xa000, 0x1000, CRC(1ddedccd) SHA1(5fc0d30b5997224b67bf286725468194359ced5a) )
	ROM_RELOAD(        0xb000, 0x1000 )
	ROM_LOAD( "203",   0x8000, 0x2000, CRC(f502175c) SHA1(cb2190e633e98586758008577265a7a2bc088233) )
	ROM_LOAD( "202",   0xc000, 0x2000, CRC(5171097d) SHA1(171999bc04dc98c74c0722b2866310d193dc0f82) )
//	ROM_CART_LOAD("cart", 0x8000, 0x8000, ROM_OPTIONAL)
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT     STATE         INIT  COMPANY    FULLNAME       FLAGS */
COMP( 1983, pencil2,   0,     0,     pencil2,   pencil2, driver_device,  0,  "Hanimex", "Pencil II", GAME_IS_SKELETON)
