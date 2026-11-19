// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

Commodore MPS-1230 Dot Matrix Printer
Commodore 1988/89
Hardware info by Guru

This is a 9-pin dot matrix printer manufactured by Commodore (possibly under license from Olivetti)
The printer is based on the Olivetti DM100 9-pin dot matrix printer with an added Commodore
serial port and other Commodore modifications. The service manual shows the PCB as 'DM10XC2',
however the actual PCB does not exactly match that shown in the service manual.
The firmware is modified by Commodore to support the CBM ASCII character and command set.

The printer comes standard with Centronics Parallel and Commodore Serial ports, sheet feed
and tractor feed and can be used with Commodore 64, 128/D, Amiga and IBM compatibles.

The ink ribbons are compatible with, and used in the following printers....
Commodore MPS 1230 printer
Olivetti DM 100, DM 90-1, DM 95, DM 99 & PR 98 printers
Birch BR-600 POS/cash register
Facit E 440 printer
Philips NMS 1016 & NMS 1432 printers
Senor Science K-700 & K-750 POS/cash register
Triumph Adler MPR 7120 printer


PCB Layout
----------

BA235
OLIVETTI OPE MADE IN HONGKONG
750746Y00-A-B9

    CENTRONICS         SERIAL
|-|------------|--------|---|----------|
|J899  J369   9306 7407 J02G      L387A|
|                                  J169|
|               D7810  6264 27512      |
|J708   MB624207                       |
|-----|                                |
      |      11.06MHz         J532     |
      |                                |
      |    555  L293  L6210 2803 2803  |
      |J404 74LS14  J477      2803 J821|
      |--------------------------------|
Notes: (all IC's shown)
      D7810    - NEC D7810HG ROM-less Microcontroller, clock input 11.06MHz  (64 pin 'spider' package)
      MB624207 - Fujitsu MB624207U MB62XXXX UHB Series 1.5um CMOS Gate Array marked 'OPE 200' (SDIP64)
      6264     - 8k x8 SRAM (DIP28)
      27512    - TMS27C512 64k x8 EPROM (DIP28)
                 S/W revisions dumped:
                                     'Release R - 1.1D 10/NOV/1988' with label 'PDL2'
                                     'Release R - 1.1E 04/APR/1989'
                                     'Release R - 2.1E 09/AUG/1989' with label 'PEEK'
      L387A    - ST L387A 5V @ 0.5A Voltage Regulator
      2803     - ULN2803A 8-Channel Darlinton Driver (DIP18)
      L6210    - ST L6210R Dual Schottky Diode Bridge (DIP16)
      L293     - ST L293ER Quad Push-Pull Driver (DIP20)
      74LS14   - National DM74LS14N Hex Inverter with Schmitt Trigger (DIP14)
      555      - Texas Instruments NE555P Timer (DIP8)
      9306     - National NMC9306N 32bytes Serial EEPROM (DIP8)
      7407     - Texas Instruments ON7404N Hex Buffer with High Voltage Open-Collector Outputs (DIP14)
      J169     - 3-pin 9v power input connector (from built-in transformer)
      J821     - 6-pin connector for paper feed motor
      J532     - 12-pin connector for print head
      J477     - 2-pin print head transport motor connector
      J404     - 4-pin connector for paper-out photosensor
      J708     - 6-pin connector for transport motor encoder
      J899     - 15-pin connector for ribbon cable for console buttons and leds (feed, online etc)
      J369     - 36-pin centronics parallel connector
      J02G     - 6-pin Commodore serial bus connector

**********************************************************************/

#include "emu.h"
#include "mps1230.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MPS1230, mps1230_device, "mps1230", "Commodore MPS-1230 NLQ Printer")


//-------------------------------------------------
//  ROM( mps1230 )
//-------------------------------------------------

ROM_START( mps1230 )
	ROM_REGION( 0x10000, "firmware", 0 )
	ROM_DEFAULT_BIOS("peek")
	ROM_SYSTEM_BIOS( 0, "pdl2", "Release R - 1.1D 10/NOV/1988" )
	ROMX_LOAD( "pdl2.f03ee", 0x00000, 0x10000, CRC(f8a9f45c) SHA1(4e7bb0d382c55432665f5576b6a5cd3c4c33bd8e), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v11e", "Release R - 1.1E 04/APR/1989" )
	ROMX_LOAD( "mps1230-v11e-xxxxxx-xx.bin", 0x00000, 0x10000, CRC(29f055db) SHA1(423799cdd9a52a71ece21fb2061d2b1844f25f6d), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "peek", "Release R - 2.1E 09/AUG/1989" )
	ROMX_LOAD( "peek.f03ee", 0x00000, 0x10000, CRC(b5215f25) SHA1(dcfdd16942652447c472301392d9b39514547af1), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mps1230_device::device_rom_region() const
{
	return ROM_NAME( mps1230 );
}


//-------------------------------------------------
//  ADDRESS_MAP( mps1230_mem )
//-------------------------------------------------

void mps1230_device::mps1230_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("firmware", 0);
	map(0xc000, 0xdfff).ram(); // as per the service manual
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mps1230_device::device_add_mconfig(machine_config &config)
{
	UPD7810(config, m_maincpu, 11'060'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mps1230_device::mps1230_mem);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mps1230_device - constructor
//-------------------------------------------------

mps1230_device::mps1230_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MPS1230, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this),
	m_maincpu(*this, "maincpu")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mps1230_device::device_start()
{
}
