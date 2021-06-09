// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli

#include "emu.h"
#include "machine/segashiobd.h"

/*
	837-14438 SH I/O BD

|-------------------------------------------------------------------------------------|
|         CN3                     CN6              CN2         CN1          CN11      |
|                                                                                     |
|                                     LED1-5                                          |
|                                                                                     |
|                DIPSW1              BT1        OSC1                                  |
|                                                                                     |
|          IC3 LED6,7 SW2                                                             |
|                                                                                     |
|                    OSC2              IC8                      IC1      IC4    IC5   |
|                                                                                     |
|CN5                                         IC7                                      |
|                                                                                     |
|                 IC2                                                                 |
|                                                                                     |
|                                                          IC6                        |
|                                                                                     |
|                                       SW1                                           |
|                                                                                     |
|                                                                    IC9              |
|                                                                          OSC3       |
|                     CN4                                   CN7        CN8            |
|-------------------------------------------------------------------------------------|

	IC1    - Hitachi/Renesas SH4 SoC
	IC2    - Xilinx Spartan XC2S50 PQ208AMS0341 FPGA
	IC3    - Xilinx 17S50APC Spartan-II Family OTP Configuration PROM, stamped 6372A
	IC4,5  - Toshiba TC59S6432CFT-10  512K x4 banks x32bit SDRAM
	IC6    - Macronix MX29LV160ATTC-90 16Mbit Flash ROM
	IC7    - ST M68AF127BL55MC6 1Mbit (128K x8) SRAM
	IC8    - 42-pin DIP socket, unpopulated
	IC9    - NS USBN9604-28M USB Node Controller
	OSC1   - 33.3333 MHz
	OSC2   - 32.0000 MHz
	OCS3   - 24.0000 MHz
	SW1,2  - pushbuttons
	DIPSW1 - 4x DIP switch
	LED1-5 - LEDs
	LED6,7 - 7seg LEDs
	BT1    - Panasonic CR2032 battery

	CN1    - 8 pin JST VH series connector
	 1 12V
	 2 5V
	 3 5V
	 4 3.3V
	 5 3.3V
	 6 gnd
	 7 gnd
	 8 gnd
	
	CN2    - 4 pin JST VH series connector
	 1 12V
	 2 12V
	 3 gnd
	 4 gnd
*/

DEFINE_DEVICE_TYPE(SEGA837_14438, sega_837_14438_device, "sega837_14438", "Sega 837-14438 SH I/O BD")

void sega_837_14438_device::sh4_map(address_map &map)
{
	// mirrors only for the debugger
	map(0x00000000, 0x001fffff).mirror(0xa0000000).rom();
	map(0x04000000, 0x0401ffff).mirror(0xa0000000).ram();
	map(0x0c000000, 0x0cffffff).mirror(0x80000000).ram();
}

#define CPU_CLOCK 200000000 // need to set the correct value here

void sega_837_14438_device::device_add_mconfig(machine_config &config)
{
	SH4LE(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_md(0, 1);
	m_maincpu->set_md(1, 0);
	m_maincpu->set_md(2, 1);
	m_maincpu->set_md(3, 0);
	m_maincpu->set_md(4, 0);
	m_maincpu->set_md(5, 1);
	m_maincpu->set_md(6, 0);
	m_maincpu->set_md(7, 1);
	m_maincpu->set_md(8, 0);
	m_maincpu->set_sh4_clock(CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &sega_837_14438_device::sh4_map);
}

sega_837_14438_device::sega_837_14438_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SEGA837_14438, tag, owner, clock),
	m_maincpu(*this, "shiobdcpu")
{
}

void sega_837_14438_device::device_start()
{
}

void sega_837_14438_device::device_reset()
{
	device_t::device_reset();
}

ROM_START(segashiobd)
	ROM_REGION(0x200000, "shiobdcpu", 0)
	ROM_LOAD("fpr-24150.ic6", 0x0000000, 0x200000, CRC(3845c34c) SHA1(027b17bac64482ee152773d5fab30fcbc6e2bcb7))    // SH4 code
	ROM_REGION(0x020000, "config_prom", 0)
	ROM_LOAD("6372a.ic3", 0x0000000, 0x020000, CRC(f30839ad) SHA1(ea1a32c4da1ed9745300bcdd7964a7c0964e3221))    // FPGA configuration prom
ROM_END

const tiny_rom_entry* sega_837_14438_device::device_rom_region() const
{
	return ROM_NAME(segashiobd);
}
