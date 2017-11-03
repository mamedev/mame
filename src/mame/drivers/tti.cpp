// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-02 Skeleton

Transitional Technology Inc., single-board computer. Model number not known, zipfile was named "TTI_10012000.zip"

Chips: NCR 53C90A, Motorola MC68901P, Fujitsu 8464A-10L (8KB static ram), and 14 undumped proms.

Other: LED, 20MHz crystal. Next to the MC68901P is another chip just as large (48 pin DIL), with a huge sticker covering all details.
       Assumed to be a Motorola CPU such as MC68000, MC68008, etc.

************************************************************************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68901.h"

class tti_state : public driver_device
{
public:
	tti_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

protected:
	required_device<cpu_device> m_maincpu;
};

static INPUT_PORTS_START( tti )
INPUT_PORTS_END

static ADDRESS_MAP_START( prg_map, AS_PROGRAM, 8, tti_state )
	AM_RANGE(0x00000, 0x07fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x78000, 0x7ffff) AM_RAM
	AM_RANGE(0x80000, 0x80017) AM_DEVREADWRITE("mfp", mc68901_device, read, write)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( tti )
	MCFG_DEVICE_ADD("maincpu", M68008, XTAL_20MHz / 2) // guess
	MCFG_CPU_PROGRAM_MAP(prg_map)

	MCFG_DEVICE_ADD("mfp", MC68901, 0)
	MCFG_MC68901_TIMER_CLOCK(XTAL_20MHz / 2) // guess
	MCFG_MC68901_RX_CLOCK(9600) // for testing (FIXME: actually 16x)
	MCFG_MC68901_TX_CLOCK(9600) // for testing (FIXME: actually 16x)
	MCFG_MC68901_OUT_SO_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("mfp", mc68901_device, write_rx))
MACHINE_CONFIG_END

ROM_START( tti )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tti_10012000_rev2.3.bin", 0x0000, 0x8000, CRC(95a5bce8) SHA1(46d7c99e37ca5598aec2062dfd9759853a237c14) )
	ROM_LOAD( "tti_10012000_rev1.7.bin", 0x0000, 0x8000, CRC(6660c059) SHA1(05d97009b5b8034dda520f655c73c474da97f822) )
ROM_END

COMP( 1989, tti, 0, 0, tti, tti, tti_state, 0, "Transitional Technology Inc", "Unknown TTI SBC", MACHINE_IS_SKELETON )
