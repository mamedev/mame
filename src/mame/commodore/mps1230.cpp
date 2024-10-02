// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    mps1230.c - Commodore MPS-1230 dot-matrix printer

    Skeleton made September, 2015 by R. Belmont

-------------------------------------

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

************************************************************************/

#include "emu.h"

#include "cpu/upd7810/upd7810.h"
#include "cpu/z80/z80.h"


namespace {

#define CPU_TAG "maincpu"

class mps1230_state : public driver_device
{
public:
	mps1230_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, CPU_TAG)
	{ }

	void mps1000(machine_config &config);
	void mps1230(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mps1230_map(address_map &map) ATTR_COLD;
};

/***************************************************************************
    PARAMETERS
***************************************************************************/

/***************************************************************************
    START/RESET
***************************************************************************/

void mps1230_state::machine_start()
{
}

void mps1230_state::machine_reset()
{
}

/***************************************************************************
    ADDRESS MAP
***************************************************************************/

void mps1230_state::mps1230_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0xc000, 0xdfff).ram(); // as per the service manual
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( mps1230 )
INPUT_PORTS_END

void mps1230_state::mps1230(machine_config &config)
{
	UPD7810(config, m_maincpu, 11060000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mps1230_state::mps1230_map);
}

void mps1230_state::mps1000(machine_config &config)
{
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mps1230_state::mps1230_map);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(mps1000)
	ROM_REGION(0x10000, "maincpu", 0)
	// ver 2.20, 06/DEC/1986 (But it could also perhaps mean 12/JUN/1986)
	// I can't tell the PCB reference because this was dumped from spare EPROMs from a drawer
	// The dump seems to be good because the data content of all of my 4 spare EPROMs matched perfectly.
	ROM_LOAD( "mps_1000_vers_2.20_12-06-86.rom", 0x000000, 0x02000, CRC(0a91ea8a) SHA1(ccb679f3d1f7f4eddb4e8899fe9e9a594dcfca5d) )
	// this is another Z80-based bios.
	ROM_LOAD( "hwh_28.06.88_ep12-2_s80r2.rom", 0x002000, 0x002000, CRC(1d17c586) SHA1(1afd1d1bbebe4d79c3cfcc30ac617b1a8ddb99e1) ) // to locate at 0000
ROM_END

ROM_START(mps1230)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "pdl2.f03ee",   0x000000, 0x010000, CRC(f8a9f45c) SHA1(4e7bb0d382c55432665f5576b6a5cd3c4c33bd8e) ) // ver 1.1D, 10/NOV/1988
	ROM_LOAD( "peek.f03ee",   0x000000, 0x010000, CRC(b5215f25) SHA1(dcfdd16942652447c472301392d9b39514547af1) ) // ver 2.1E, 09/AUG/1989
ROM_END

} // anonymous namespace


/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY                        FULLNAME */
SYST( 1986, mps1000, 0,      0,      mps1000, mps1230, mps1230_state, empty_init, "Commodore Business Machines", "MPS-1000 Printer",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
SYST( 1988, mps1230, 0,      0,      mps1230, mps1230, mps1230_state, empty_init, "Commodore Business Machines", "MPS-1230 NLQ Printer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
