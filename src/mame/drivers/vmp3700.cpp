// license:GPLv2+
// copyright-holders:publicdomain
#include "emu.h"
#include "cpu/i386/i386.h"

/*
CPU Board:

U1  BRIGHT BM29F040-90NC                    | 4 MEGABIT (512K × 8) 5 VOLT SECTOR ERASE CMOS FLASH MEMORY
U5  NEW2001-16 1B9285LT M002646             | (custom / ROM ?)
U4  HB993200-32                             | (custom / ROM ?)
U2  EliteMT LP621024DM-70LL 0021S B3P14BA   | 128K X 8 BIT CMOS SRAM
U3  EliteMT LP621024DM-70LL 0021S B3P14BA   | 128K X 8 BIT CMOS SRAM
U10 Motorola AC139 XAA021 (16pin smd)       | ? (DUAL 1-OF-4 DECODER/DEMULTIPLEXER ?)
U7  Intel KU80386EX33                       | Intel 80386 EX Embedded Microprocessor
U11 XTAL 40.500MHz JTC                      | 40.5MHz crystal 
U6  XTAL 48.000MHz JTC                      | 48MHz crystal
U13 TMTECH UAP49MR 0025 83MHz T224162B-35J  | 256K x 16 DYNAMIC RAM
U15 Winbond W9925QF-K                       | VCD (Video CD) Decoder 
U16 Winbond W9952QP                         | TV Encoder
U12 ADC VISA99 0023E (160pin)               | (custom ?)
U17 TMTECH UAP49MR 0025 83MHz T224162B-35J  | 256K x 16 DYNAMIC RAM
U18 TMTECH UAP49MR 0025 83MHz T224162B-35J  | 256K x 16 DYNAMIC RAM
U9  LS04                                    | 6 x Boolean NOT gate
U14 Motorola MC1378P CTCG0029               | Motorola - Color Television Composite Video Overlay Synchronizer
U19  ?????                                  | ?

Audio Module Board:

?
*/

class vmp3700_state : public driver_device
{
public:
	vmp3700_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};

//          ADH  ADL |   MSKH MSKL |                address | SMM | bits | MEM | RDY | RES. | WAIT STATE
// CS0:  0x0000 0503 | 0x0001 FC01 |     000 << 11 = 000000 |   1 |    8 |   1 |   0 |   00 | 00011
// CS1:  0x0002 0503 | 0x0001 FC01 |     400 << 11 = 200000 |   1 |    8 |   1 |   0 |   00 | 00011
// CS2:  0x0006 8503 | 0x0000 7C01 |     C10 << 11 = 610000 |   1 |    8 |   1 |   0 |   00 | 00011
// CS3:  0x0004 0783 | 0x0000 FC01 |     800 << 11 = 400000 |   1 |   16 |   1 |   1 |   00 | 00011
// CS4:  0x0007 0503 | 0x0000 FC01 |     E00 << 11 = 700000 |   1 |    8 |   1 |   0 |   00 | 00011
// CS5:  0x0005 0503 | 0x0000 FC01 |     A00 << 11 = 500000 |   1 |    8 |   1 |   0 |   00 | 00011
// CS6:  0x0006 0503 | 0x0000 7C01 |     C00 << 11 = 600000 |   1 |    8 |   1 |   0 |   00 | 00011
// UCS:  0x0008 0503 | 0x0007 FC01 |    1000 << 11 = 800000 |   1 |    8 |   1 |   0 |   00 | 00011
//SS:SP => C9E0 + 0400 = 0xCDE0

static ADDRESS_MAP_START(vmp3700_map, AS_PROGRAM, 16, vmp3700_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x01ffff) AM_RAM //CS0 128k SRAM at U2
	AM_RANGE(0x080000, 0x0fffff) AM_ROM AM_REGION("bios", 0) //UCS 512k
	AM_RANGE(0x200000, 0x21ffff) AM_RAM //CS1 128k SRAM at U3
//	AM_RANGE(0x400000, 0x40ffff) AM_? //CS3 64k U15-pin3 (Winbond W9925QF-K)
	AM_RANGE(0x500000, 0x50ffff) AM_ROM AM_REGION("maskrom_u5", 0) //CS5 64k U5-pin12 MX23C6410
//	AM_RANGE(0x600000, 0x607fff) AM_? //CS6 32k
//	AM_RANGE(0x610000, 0x617fff) AM_? //CS2 32k
	AM_RANGE(0x700000, 0x70ffff) AM_ROM AM_REGION("maskrom_u4", 0) //CS4 64k U4-pin12 MX23C6410
	AM_RANGE(0x800000, 0x87ffff) AM_MIRROR(0x3780000) AM_ROM AM_REGION("bios", 0) //UCS 512k
ADDRESS_MAP_END

/*
MaskROM pinout (for bankswitching):
   A17 = P3.2 pin 80
   A18 = P3.3 pin 82
   A19 = P3.4 pin 84
   A20 = P3.5 pin 85
U4 A21 = P3.6 pin 86

*/

static MACHINE_CONFIG_START( vmp3700, vmp3700_state )
	MCFG_CPU_ADD("maincpu", I386EX, XTAL_48MHz / 2)
	MCFG_CPU_PROGRAM_MAP(vmp3700_map)
	//MCFG_CPU_IO_MAP(vmp3700_io)
MACHINE_CONFIG_END

ROM_START( vmp3700 )
	ROM_REGION(0x80000,"bios", 0)
	ROM_LOAD("vmp3700.u1", 0x00000, 0x80000, CRC(a98ee764) SHA1(10352257f6be9e5053cc05e39932efd8de4f89a8))

	ROM_REGION(0x800000,"maskrom_u4", 0)
	ROM_LOAD("hb993200-32.u4", 0x00000, 0x800000, NO_DUMP)

	ROM_REGION(0x800000,"maskrom_u5", 0)
	ROM_LOAD("new2001-16.u5", 0x00000, 0x800000, NO_DUMP)
ROM_END

/*    YEAR  NAME     PARENT  COMPAT   MACHINE  INPUT                 INIT     COMPANY            FULLNAME */
COMP( 1998, vmp3700,      0,      0,  vmp3700,     0, driver_device,    0,    "Raf Electronics", "Vmp 3700 Videoke", MACHINE_NO_SOUND)
