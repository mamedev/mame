// license:BSD-3-Clause
// copyright-holders:
/*

Skeleton driver for Recreativos Franco PC-Pentium based video + mechanical slots machines.

TODO:
- Keeps reading $fed40000 (mapped thru PCI), expects to read with bit 7 high.
- In shutms11 it will start booting Win CE "BIOS" bootloader then quickly Triple Fault
  at $1d/$1e****** range check.

Configuration for 'Santa Fe Golden' video slot machine:

Operating system
  Microsoft Windows Embedded CE 6.0

Motherboard
  Advantech AIMB-256
  Intel Celeron 575 1M cache, 2,00 GHz
  1GB RAM
  Compact Flash Transcend 512MB 20100202

Advantech AIMB-256 is a single board PCB with:
- Intel ICH8-M
- Intel GME965 (GMCH X3100)
- Realtek RTL8111B (LAN, PCIe x 2)
- Realtek ALC888 (HD Audio, 7.1+2 channels)
- W83627DHG Super I/O LPC i/f
- Optional infineon SLB9635TT on LPC bus (TPM 1.2)
- Compact Flash support
- VGA + DVI ports
- 10 USB ports
- DIO
- PCI1 connector
There's another version of "Santa Fe Golden" that uses a "SBC81870" industrial
PC motherboard (Pentium M, Intel 855GME+ICH4, Winbond W83627HG-AW, Intel 855GME),
but it came with a different image on the Compact Flash (currently undumped).

PCB coinage (monedero-billetero-hopper) R.F. 53452303
  PIC18F448-I/P
  ULN2803A
  Xtal 12.000 MHz
  SEEPROM PCA82C251
  3xHC573A
  74HC238D
     ______________________________________________
    | .... .... .... ........... .... .... ... .. |
    |          Xtal                               |
    |        12.000 MHz       ___               ..|
    |  _____________________ 82C251  ___        ..|
    | | PIC18F448-I/P      |        |  |<-ULN2803A|
    | |____________________| ·····  |  |        ..|
    | __     ______  ______  ______ |  |        ..|
    74HC238D HC573A  HC573A  HC573A |__|     :  ..|
    |   _______  _______  _______            :  ..|
    |  | L6203 || L6203 || L6203 |           :  ..|
    |                                        :  ..|
    | ..........  ..........  ..........          |
    |_____________________________________________|


PCB lower lights and buttons (luces inferior y botonera) R.F. 53446002 LF
  PIC18F4480-I/P
  Xtal 12.000 MHz
      _________________________________________
    _| SIDE 1                                  |_
   |                                            |
   |                                            |
   |  A82C251 Xtal  TPIC6595  TPIC6595          |
   |         12 MHz     TPIC6595   TPIC6595     |
   |    __             _                   __   |
   |___|  |_|_|_|_|_|_| |_|_|_|_|_|_|_|_|_|  |__|

      _________________________________________
    _| SIDE 2         ____________________    |_
   |     .....       | PIC18F4480-I/P    |      |
   |                 |___________________|      |
   |                                       U1   |
   |                              NOT POPULATED |
   |    __                   _             __   |
   |___|  |_|_|_|_|_|_|_|_|_| |_|_|_|_|_|_|  |__|


PCB upper lights (luces superior) R.F. 53446002 LF
  PIC18F4480-I/P
  Xtal 12.000 MHz
      _________________________________________
    _| SIDE 1                                 |_
   |                                            |
   |                                            |
   |  A82C251 Xtal  TPIC6595  TPIC6595          |
   |         12 MHz     TPIC6595   TPIC6595     |
   |    __             _                   __   |
   |___|  |_|_|_|_|_|_| |_|_|_|_|_|_|_|_|_|  |__|

      _________________________________________
    _| SIDE 2         ____________________    |_
   |     .....       | PIC18F4480-I/P    |      |
   |                 |___________________|      |
   |                                       U1   |
   |                              NOT POPULATED |
   |    __                   _             __   |
   |___|  |_|_|_|_|_|_|_|_|_| |_|_|_|_|_|_|  |__|


PCB reels (rodillos) R.F. 53435604 LF
  PIC 18F2480
  Xtal 12.000 MHz
     ____________________________________
    |LED->o                             |
    |:                  __________      |
    |:                  |ULN2803L|      |
    |:          __________             :|
    |:         |TPIC6595N|             :|
    |:          __________             :|
    |:         |TPIC6595N|     __      :|
    |:                        |  |<-TPIC6B595N
    |:          __________    |  |     :|
    |:         |TPIC6595N|    |  |     :|
    |:          __________    |__|      |
    |          |TPIC6595N|              |
    |:                                  |
    |:                                 :|
    |:          __________     __      :|
    |:         |TPIC6595N|    |  |<-ULN2803L
    |:          __________    |  |     :|
    |:         |TPIC6595N|    |  |     :|
    |:                        |__|     :|
    |                                   |
    |:          __________              |
    |:         |TPIC6595N|             :|
    |:      MM74HC04M                  :|
    |:              74HC140 74HC140    :|
    |:       ···  ________________     :|
    |:           |__PIC18F2480___|     :|
    |            ___    Xtal           :|
    |          MCP2551  12.000 MHz     :|
    |    ....  ...  .............       |
    |___________________________________|


PCB Sound R.F. 53422409 LF
  Empty sockets: SND1 U22 (40 pin), HIGH U1 (32 pin), LOW U3 (32 pin), RAM U6 (32 pin)
  OKI M6650
  Xtal 4.000 MHz
  AMD N80C188-20
  Xtal 32.000 MHz
    _________________________________________________
   | __________________ ····     (o)    ___   o o    |
   || U22 EMPTY       | 24LC64         |___<-4xDSW   |
   ||_________________|    _______  74HC238D        :|
   |    _________        R8A66153FP  _________  ___ :|
   |   |OKI     |                   |SJA1000T| |__|<-A82C251
   |   |M6650   |         ___       ___    ___      :|
   |   |________|        |  |      |  |   |  |<-U19 EMPTY
   |  _______  Xtal      |  | 4xDSW->_|   |__|      :|
   | |_LM380N| 4.000 MHz |__<-8xDSW                 :|
   |           ..  ..   ..     _____    Xtal 74HC08D |
   |   _____  _____  _____    HC573A  32.00 MHz      |
   |..|U1  | |U3  | |U6  |       ___________         |
   |..|EMPTY |EMPTY |EMPTY      |AMD       |  LS04   |
   |: |HIGH| |LOW | |RAM |      |N80C188-20|         |
   |: |    | |    | |    |      |          |  LS02   |
   |HC573A | |    | |    |      |__________|         |
   |  |____| |____| |____|                    NE555  |
   |_________________________________________________|


PCB ccTalk R.F. 53475502
  PIC18F2580
  Xtal 10.000 MHz
  SEEPROM 82C251Y
    _________________________
    | ··   .... CAN ....     |
    | ..           82C251Y  :|
    |   _____________       :|
    |: |_PIC18F2580_|  ccTalk|
    |:        Xtal          :|
    |:       10.000 MHz     :|
    |________________________|


PCB counters (contadores) R.F. 53430106
  PIC18F2580
  Xtal 12.00 MHz
  SEEPROM 82C251
    ___________________________________
    | :  ·····      __________  :  ··  |__
    |    Xtal 12.00 |ULN2803A|  :  :      |
    |  _____     _____________     ·      | <-DB9
    |  A4N46    |_PIC18F2580_|   ______   |
    | . ...  .. .  ______ ______ MAX232 __|
    |              MAX232 82C251  ···· |
    |__________________________________|
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"

namespace {

class rfslotspcpent_state : public driver_device
{
public:
	rfslotspcpent_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void rfslotspcpent(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void rfslotspcpent_map(address_map &map) ATTR_COLD;
};

void rfslotspcpent_state::rfslotspcpent_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x1e0000);
	map(0xffe00000, 0xffffffff).rom().region("bios", 0);
}

static INPUT_PORTS_START( rfslotspcpent )
INPUT_PORTS_END

void rfslotspcpent_state::rfslotspcpent(machine_config &config)
{
	// Socket 478 Core 2 Duo / Celeron M
	PENTIUM4(config, m_maincpu, 100'000'000); // Actually an Intel Celeron 575 1M cache, 2,00 GHz
	m_maincpu->set_addrmap(AS_PROGRAM, &rfslotspcpent_state::rfslotspcpent_map);

	PCI_ROOT(config, "pci", 0);
	// ...
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( rfsantafeg )
	ROM_REGION32_LE(0x200000, "bios", 0) // Advantech AIMB-256
	ROM_LOAD("bios_a256v103.bin", 0x000000, 0x200000, CRC(06df0d8d) SHA1(5d740071500729af8c045b562adc5f8da058b59f) )

	DISK_REGION( "ide:0:hdd" ) // Compact Flash Transcend 512MB
	DISK_IMAGE( "santa_fe_golden_g12v5_11482e_advantech", 0, SHA1(7ad57f0c80e89ab8086fc5c9c7ba9be4b2c90451) )

	ROM_REGION(0x4000, "pics", 0)

	/*
	PCB counters (contadores) R.F. 53430106
	-PIC18F2580
	-Xtal 12.00 MHz
	-SEEPROM 82C251
	*/
	ROM_LOAD("pic_contadores_15112a_ips_pic18f2580.bin",                      0x00000, 0x1000, NO_DUMP ) // 32K Flash

	/*
	PCB lower lights and buttons (luces inferior y botonera) R.F. 53446002 LF
	-PIC18F4480-I/P
	-Xtal 12.000 MHz
	*/
	ROM_LOAD("pic_luces_inferior_y_botonera_11242a_310d64_pic18f4480-i-p.u2", 0x01000, 0x0800, NO_DUMP ) // 16K Flash

	/*
	PCB upper lights (luces superior) R.F. 53446002 LF
	-PIC18F4480-I/P
	-Xtal 12.000 MHz
	*/
	ROM_LOAD("pic_luces_superior_11125c_2f05c0_pic18f4480-i-p.u2",            0x01800, 0x0800, NO_DUMP ) // 16K Flash

	/*
	PCB coinage (monedero-billetero-hopper) R.F. 53452303
	-PIC18F448-I/P
	-ULN2803A
	-Xtal 12.000 MHz
	-SEEPROM PCA82C251
	-3xHC573A
	-74HC238D
	*/
	ROM_LOAD("pic_monedero_billetero_hopper_09482b_0eda_pic18f448-i-p.u2",    0x02000, 0x0800, NO_DUMP ) // 16K Flash

	/*
	PCB ccTalk R.F. 53475502
	-PIC18F2580
	-Xtal 10.000 MHz
	-SEEPROM 82C251Y
	*/
	ROM_LOAD("pic_placa_cctalk_pic18f2580.u1",                                0x02800, 0x1000, NO_DUMP ) // 32K Flash

	/*
	PCB reels (rodillos) R.F. 53435604 LF
	-PIC 18F2480
	-Xtal 12.000 MHz
	*/
	ROM_LOAD("pic_rodillos_14491a_ic8693.u1",                                 0x03800, 0x0800, NO_DUMP ) // 16K Flash
ROM_END

} // Anonymous namespace


GAME( 2014, rfsantafeg, 0, rfslotspcpent, rfslotspcpent, rfslotspcpent_state, empty_init, ROT0, "Recreativos Franco", "Santa Fe Golden", MACHINE_IS_SKELETON_MECHANICAL )
