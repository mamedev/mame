// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Aristocrat MK7 "Viridian WS" (circa ~2006)

TODO:
- Throws "MK7i FPGA not found" & "Smartcard init failed" in shutms11 when bootstrapping with custom
  GRUB;

Notes:
- Games (mostly) runs on either CF or SATA ext2 partitions (heuristics TBD);
- PCIe version 2.0;
- Each game requires their own SAM modules (unknown type(s) at current time, TBD),
  runs Java code inside. cfr. ISO/IEC 7816;

===================================================================================================

Hardware info by Guru
----------------------

This is a gambling system comprising several custom PC-like boards
plugged into a backplane. The games are not particularly CPU intensive.
The sole purpose of the complexity of these custom boards is to prevent
bootlegging, game swapping and repairing.

Main board
----------

Sticker: MK7 CARRIER BOARD Assembly:494077_01
On Bottom: MK7i Carrier Board Issue B PCB 494076 (C) 2006
|------------------------------------------------------------------------------|
|                 J1                  J2                 J3                    |
|  ALC883                                                             ST2044B  |
|                                                                     ST2044B  |
|                                                                              |
|                                                                              |
|     FCFG       L9823                                                         |
|                 l9823  L9823          UDQ2559                                |
|    J10                                                            LM5642     |
| |-|                                                                          |
| | |                                                                          |
| | |                                                                          |
| | |                            |--------|                                    |
| | |                            |ALTERA  |                                    |
| | |                            |CYCLONE |     M5M5W816                       |
| | |                            |  II    |          M5M5W816                  |
| | |                            |--------|               M5M5W816             |
| | |                                                                          |
| | |                                                                          |
| | |               |-------------------|        EPM570                        |
| | |               |-------------------|                                      |
| | |                        J4                          LM339       49150     |
| | |   |-|         |-------------------|          LM385    LM339              |
| | |   | |         |-------------------|          DS2401                      |
| | |   | |                                             J13 J14          BT1   |
| |-|   |-|J22       SATA2 SATA1                            J15       BT2      |
| x16   x1     BIOS     J6  J9       |---------|                               |
|                 SYS_CFG    |---|   |   CF    |  PROM1 PROM2                  |
|PI6C20400                   |SC |   |         |                      7LED 7LED|
|-------------RJ45-----------|---|---|---------|----USBA------DE9--------------|
Notes: (many power-related parts such as resistors (R), inductors (L), capacitors (C) and switching
        regulators are not shown)
      J1/J2/J3 - Multi-pin connector (total 330 pins)
        ALC883 - Realtek ALC883 7.1+2 Channel High Definition Audio Codec
          FCFG - AT17F040A PLCC20 FPGA Configuration Flash Memory with label '590175_01 1.0002' at
                 U2. PCB marking 'FCFG'
        LM5642 - Texas Instruments LM5642 High Voltage Dual Synchronous Buck Converter
    CYCLONE II - Altera Cyclone II FPGA EP2C20F484C8N
        EPM570 - Altera EPM570F100C4N CPLD with sticker '590177 v.03'
         L9823 - ST Microelectronics ST L9823 Low-Side Octal Driver
       UDQ2559 - Quad Power Driver
         49150 - Microchip MIC49150 1.5A LDO Regulator
       ST2044B - ST Microelectronics ST2044B Power Distribution Switch
           x16 - PCIe-x16 slot. A video board may plug in here but the type is unknown.
                 A simple Dual DVI board came with the motherboard and that might plug in here and
                 the video board plugs into another slot/carrier board, possibly
                 Viridian video card nVidia GeForce GT430 (GF108).
                 The dual DVI board has sticker 'WAAD00010DV2-SA ARI P/N 432660 Dual DVI ADD2 802-0017973'
                 and contains 2x Silicon Image SiL1364ACNU chips. This simply sends the video to a
                 DVI connector. If the on-board
                 Intel 945 is used then the video comes out via the Dual DVI card.
            x1 - PCIe-x1 slot
         LM339 - Texas Instruments LM339 Quad Differential Comparator
      M5M5W816 - Mitsubishi M5M5W816WG-55 8388608-bit (524288-word x16-bit) CMOS Static RAM
         BT1/2 - Panasonic BR-2/3A 1/2AA 3V Lithium Battery (x2)
  J10/13/14/15 - JTAG connectors
         LM385 - Texas Instruments LM385 Voltage Reference
        DS2401 - Dallas DS2401 Silicon Serial Number with 64-bit ROM
            J4 - 110 pin connector (x2) for CPU Module connection
           J22 - Fan 2-pin header
     PI6C20400 - Diodes Incorporated PI6C20400HE Clock Buffer 1:4 PCI Express Clock Driver
          RJ45 - RJ45 Ethernet Network Connector, XMULTIPLE XMH-TRJG 1730AONL
          BIOS - SST 49LF008A 8M-bit (1Mx8-bit) NOR Flash ROM at location U1 with sticker
                 '590176_01_1.00.7' with additional silver 'Phoenix Technologies' sticker.
       SATA1/2 - Standard SATA Data Cable Connectors for HDD (game code storage and usage
                 varies per game)
          J6/9 - SATA Custom Power Connector
            CF - Standard Compact Flash Slot (game code storage and usage varies per game)
            SC - Standard 25mm x 15mm SIM Card Slot (for additional unique-per-game protection)
          USBA - Standard single USB-A Connector
           DE9 - 9 pin DE9 Serial Port
          7LED - 7-Segment LED (x2)
       SYS_CFG - Atmel AT24C02 256-byte EEPROM at location U3. Top of chip custom / obfuscated
                 markings: 'ATMLU012 08B 1'. Sticker: 'A37'
       PROM1/2 - Atmel AT24C01 128-byte EEPROM at location U4/U5. Top of chip custom / obfuscated
                 markings: 'ATMLU010 02B 1'. No stickers on these and blank / unused.


CPU Module (plugs into main board J4)
----------
LY30C883
Sticker: ARI P/N 432394_1 MSC P/N ARI CXE-CD945-440-065007-HS_1
|-------------------------------------------------|
|14.31818MHz |------------|                       |
|CY28411ZXC  |   INTEL    |   |---------|         |
|CY25823ZXC  | Q682945GME |   |         |------|  |
|            |   SLA9H    |   |         |      |  |
||----------||            |   |         |      |  |
||  INTEL   ||            |   | SODIMM  |      |  |
||NH82801GBM||------------|   |         |SODIMM|  |
||  SL8YB   |                 |  512MB  |      |  |
||          ||------------|   |         | 512MB|  |
||----------||   INTEL    |   |         |      |  |
|            |LE80538 440 |   |         |      |  |
|  32.768kHz |  L904A875  |   |         |      |  |
|PC82573L    |   SL9LF    |   |         |      |  |
|         X  |1.86/1M/533 |   |---------|      |  |
|       25010|------------|          |---------|  |
|-------------------------------------------------|
Notes:
         X - location for TSOP48 flash ROM, not populated
     25010 - Atmel 25010 128-byte serial EEPROM
     SLA9H - Intel 82945 Memory / Video Controller (northbridge + Calistoga GPU)
     SL9LF - Intel Celeron-M 440 CPU (1.86GHz / 1M cache / 533 FSB). Clock 266 * 7 = 1862MHz
  PC82573L - Intel PC82573L Gigabit Ethernet Controller with 25MHz OSC on bottom side of PCB
   NH82801 - Intel NH82801GBM I/O Controller (southbridge: SATA, USB2.0 etc)
CY28411ZXC - Cypress (now Infineon) CY28411ZXC Clock Generator with 266MHz output
CY25823ZXC - Cypress (now Infineon) CY25823ZXC PLL (Phase Lock Loop)

Other parts on bottom: (mostly power-related)
  ISL6227CAZ - Intersil (now Renesas) ISL6227CAZ Dual PWM Controller (x2)
       FDS70 - Dual Switching Diode (6 on PCB at various locations)
       4816B - N-channel Power MOSFET
 SLB9635TT12 - Infineon TPM1.2 IC with connected 32.768kHz oscillator
       LM339 - Texas Instruments LM339 Quad Differential Comparator
      12F509 - Microchip PIC12F509 8-bit Flash Microcontroller marked '28181 V1.2'
    W83L7866 - Winbond W83L7866 Hardware Monitor IC (connected to PIC12F509)
 ATMLH00804B - Atmel AT24C02 256-byte EEPROM with custom / obfuscated markings located on bottom
               near RAM slots
  ISL6262CRZ - Intersil (now Renesas) ISL6262CRZ Two-Phase Buck Converter
663674140945 \ ? in silver tin-can enclosed shell. Two of each present on PCB
663582190951 / and located near ISL6262

Apparently two PCB revisions, one is red coated.

**************************************************************************************************/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class aristmk7_state : public driver_device
{
public:
	aristmk7_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void aristmk7(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	void aristmk7_io(address_map &map) ATTR_COLD;
	void aristmk7_map(address_map &map) ATTR_COLD;
};


void aristmk7_state::aristmk7_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0xe0000);
	map(0xfff00000, 0xffffffff).rom().region("bios", 0);
}

void aristmk7_state::aristmk7_io(address_map &map)
{
}

static INPUT_PORTS_START(aristmk7)
INPUT_PORTS_END

void aristmk7_state::aristmk7(machine_config &config)
{
	PENTIUM3(config, m_maincpu, 266'000'000 * 7); /* Celeron-M 440, 533 MHz FSB, 1.86 GHz, actual equation TBD */
	m_maincpu->set_addrmap(AS_PROGRAM, &aristmk7_state::aristmk7_map);
	m_maincpu->set_addrmap(AS_IO, &aristmk7_state::aristmk7_io);

	PCI_ROOT(config, "pci", 0);
	// ...
}

// Phoenix based BIOS
#define ARISTMK7_BIOS \
	ROM_REGION32_LE(0x100000, "bios", 0) \
	ROM_LOAD( "mk7-bios_590176_01_1.00.7.u1", 0x000000, 0x100000, CRC(f2fea07e) SHA1(285038473fbe3a5708045e2f8b14562f3c24b203) )


ROM_START( aristmk7 )
	ARISTMK7_BIOS
ROM_END

ROM_START( a7lucky88 )
	ARISTMK7_BIOS

	// compact flash
	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "l88", 0, SHA1(5734872adc867f10ee21790e544efb260fd3d266) )

	ROM_REGION( 0x10000, "sam", ROMREGION_ERASEFF )
	ROM_LOAD( "sam.bin", 0, 0x10000, NO_DUMP )
ROM_END

} // anonymous namespace


GAME(2006?, aristmk7, 0, aristmk7, aristmk7, aristmk7_state, empty_init, ROT0, "Aristocrat", "Aristocrat MK-7 BIOS", MACHINE_IS_SKELETON | MACHINE_IS_BIOS_ROOT )

GAME(200?,  a7lucky88, aristmk7, aristmk7, aristmk7, aristmk7_state, empty_init, ROT0, "Aristocrat", "Lucky 88 (Aristocrat MK-7)", MACHINE_IS_SKELETON )
