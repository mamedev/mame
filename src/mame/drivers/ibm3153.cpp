// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

IBM 3153 Terminal.

2016-05-04 Skeleton driver.

A green-screen terminal with a beeper.
Chip complement:
U1    K6T0808C10-0870 (32k static ram)
U2    D-80C32-16 (cpu)
U3    DM74LS373N
U5    LM339N
U6    DM74LS125AN
U7    K6T0808C10-0870 (ram)
U8    K6T0808C10-0870 (ram)
U9    598-0013040 6491 3.19 (boot rom)
U10   DS1488N
U11   74LS377N
U12   DS1489AN
U13   LSI VICTOR 006-9802760 REV B WDB36003 Y9936 (video processor)
U14   74F00PC
U16   DM74LS125AN
U17   DS1488N
U18   DS1489AN
U25   SN74F04N
U100  74F07N
Crystals:
Y1    16.000 MHz
Y2    65.089 MHz
Y3    44.976 MHz


ToDo:
- Everything!

****************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"

class ibm3153_state : public driver_device
{
public:
	ibm3153_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	const UINT8 *m_p_chargen;
	DECLARE_PALETTE_INIT(ibm3153);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
private:
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START(ibm3153_mem, AS_PROGRAM, 8, ibm3153_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000,0x3ffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(ibm3153_io, AS_IO, 8, ibm3153_state)
	AM_RANGE(0x0000,0xffff) AM_RAM
	//ADDRESS_MAP_UNMAP_HIGH
	//ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( ibm3153 )
INPUT_PORTS_END

UINT32 ibm3153_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

PALETTE_INIT_MEMBER( ibm3153_state, ibm3153 )
{
	palette.set_pen_color(0, 0, 0, 0 ); /* Black */
	palette.set_pen_color(1, 0, 255, 0 );   /* Full */
	palette.set_pen_color(2, 0, 128, 0 );   /* Dimmed */
}

void ibm3153_state::machine_reset()
{
	m_p_chargen = memregion("chargen")->base();
}

static MACHINE_CONFIG_START( ibm3153, ibm3153_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80C32, XTAL_16MHz) // no idea of clock
	MCFG_CPU_PROGRAM_MAP(ibm3153_mem)
	MCFG_CPU_IO_MAP(ibm3153_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(ibm3153_state, screen_update)
	MCFG_SCREEN_SIZE(640, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 239)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(ibm3153_state, ibm3153)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ibm3153 )
	ROM_REGION( 0x40000, "user1", 0 )
	ROM_LOAD("598-0013040_6491_3.19.u9", 0x0000, 0x040000, CRC(7092d690) SHA1(a23a5bd5eae90e9b31fa32ef4be1258612eaaa0a) )

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "char.bin", 0x0000, 0x2000, NO_DUMP ) // probably inside the video processor
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE   INPUT    CLASS          INIT  COMPANY            FULLNAME       FLAGS */
COMP( 1999?, ibm3153, 0,      0,       ibm3153,  ibm3153, driver_device,  0,  "IBM", "IBM 3153 Terminal", MACHINE_IS_SKELETON)
