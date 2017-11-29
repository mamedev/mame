// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for Micro-Term terminals.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/mc2661.h"
#include "machine/mc68681.h"
//#include "video/scn2674.h"
//#include "screen.h"

class microterm_state : public driver_device
{
public:
	microterm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		//, m_p_chargen(*this, "chargen")
	{ }

	DECLARE_READ8_MEMBER(c000_r);

private:
	required_device<cpu_device> m_maincpu;
	//required_region_ptr<u8> m_p_chargen;
};

READ8_MEMBER(microterm_state::c000_r)
{
	return machine().rand() & 0x80;
}

static ADDRESS_MAP_START( mt420_mem_map, AS_PROGRAM, 8, microterm_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0xc000, 0xc000) AM_READ(c000_r) AM_WRITENOP
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mt420_io_map, AS_IO, 8, microterm_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xe0, 0xef) AM_DEVREADWRITE("duart", scn2681_device, read, write)
	AM_RANGE(0xf0, 0xf3) AM_DEVREADWRITE("asci", mc2661_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mt5510_mem_map, AS_PROGRAM, 8, microterm_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("maincpu", 0) AM_WRITENOP
	AM_RANGE(0x8000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mt5510_io_map, AS_IO, 8, microterm_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x60, 0x6f) AM_DEVREADWRITE("duart", scn2681_device, read, write)
ADDRESS_MAP_END

static INPUT_PORTS_START( microterm )
INPUT_PORTS_END

static MACHINE_CONFIG_START( mt420 )
	MCFG_CPU_ADD("maincpu", Z80, 4'000'000)
	MCFG_CPU_PROGRAM_MAP(mt420_mem_map)
	MCFG_CPU_IO_MAP(mt420_io_map)

	MCFG_DEVICE_ADD("duart", SCN2681, XTAL_3_6864MHz) // MC2681
	MCFG_MC68681_IRQ_CALLBACK(INPUTLINE("maincpu", 0))
	MCFG_MC68681_OUTPORT_CALLBACK(DEVWRITELINE("eeprom", eeprom_serial_93cxx_device, di_write)) MCFG_DEVCB_BIT(5)
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("eeprom", eeprom_serial_93cxx_device, cs_write)) MCFG_DEVCB_BIT(4)
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("eeprom", eeprom_serial_93cxx_device, clk_write)) MCFG_DEVCB_BIT(3)

	MCFG_DEVICE_ADD("asci", MC2661, XTAL_3_6864MHz) // SCN2641

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_EEPROM_SERIAL_DO_CALLBACK(DEVWRITELINE("duart", scn2681_device, ip6_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( mt5510 )
	MCFG_CPU_ADD("maincpu", Z80, XTAL_6MHz)
	MCFG_CPU_PROGRAM_MAP(mt5510_mem_map)
	MCFG_CPU_IO_MAP(mt5510_io_map)

	MCFG_DEVICE_ADD("duart", SCN2681, XTAL_3_6864MHz)
	MCFG_MC68681_IRQ_CALLBACK(INPUTLINE("maincpu", 0))
	MCFG_MC68681_OUTPORT_CALLBACK(DEVWRITELINE("eeprom1", eeprom_serial_93cxx_device, di_write)) MCFG_DEVCB_BIT(6)
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("eeprom2", eeprom_serial_93cxx_device, di_write)) MCFG_DEVCB_BIT(5)
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("eeprom1", eeprom_serial_93cxx_device, cs_write)) MCFG_DEVCB_BIT(4)
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("eeprom2", eeprom_serial_93cxx_device, cs_write)) MCFG_DEVCB_BIT(4)
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("eeprom1", eeprom_serial_93cxx_device, clk_write)) MCFG_DEVCB_BIT(3)
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("eeprom2", eeprom_serial_93cxx_device, clk_write)) MCFG_DEVCB_BIT(3)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom1")
	MCFG_EEPROM_SERIAL_DO_CALLBACK(DEVWRITELINE("duart", scn2681_device, ip6_w))

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom2")
	MCFG_EEPROM_SERIAL_DO_CALLBACK(DEVWRITELINE("duart", scn2681_device, ip5_w))
MACHINE_CONFIG_END


/**************************************************************************************************************

Micro-Term Model 420.
Chips: Z80, MC2681P, SCN2674, 2x CDM6264E3, TMM2016BP-12, SCN2641, NMC9345N. Undumped PAL10L8NC at U18 and PROM (N82S129N) at U41.
Crystals: 3.6864, 15.30072 (hard to read), 9.87768

***************************************************************************************************************/

ROM_START( mt420 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "1910_M.P._R1.9.u8",   0x0000, 0x8000, CRC(e79154e9) SHA1(7c3f22097b931986c921bf731de98a1d0536aec9) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "mt420cg_rev2.1.u44",  0x0000, 0x0fe0, CRC(7950e485) SHA1(1f03525958464bbe861d2e78f07cc5264e17c0e8) ) // incomplete?
ROM_END



/**************************************************************************************************************

Micro-Term 5510.
Chips: Z80, SCN2681, S8842C4/SCX6244UNT, 4x CXK5864BP-70L, 2x NMC9346N
Crystals: 6.000, 3.68640, 45.8304

***************************************************************************************************************/

ROM_START( mt5510 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "2500_M.P._R1.9.u11", 0x00000, 0x10000, CRC(71f19a53) SHA1(91df26d46a93359cd033d7137f1676bcfa58223b) )
ROM_END




COMP( 1986, mt420, 0, 0, mt420, microterm, microterm_state, 0, "Micro-Term", "Micro-Term 420", MACHINE_IS_SKELETON )
COMP( 1988, mt5510, 0, 0, mt5510, microterm, microterm_state, 0, "Micro-Term", "Micro-Term 5510", MACHINE_IS_SKELETON )
