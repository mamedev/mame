// license:BSD-3-Clause
// copyright-holders:smf
/*
 * Konami 573 Network PCB Unit
 *
 */

#include "k573npu.h"

/*

  System 573 Hard Drive and Network Unit
  --------------------------------------

  This box is used with later Drum Mania and Guitar Freaks (possibly 9 to 11)

  PCB Layout
  ----------

  PWB0000100991 (C)2001 KONAMI
  |--------------------------------------------------------------------------|
  |    CN1               MB3793     74HC14          FLASH.24E       RJ45     |
  |                                                                          |
  |    LCX245                               DIP40                         CN3|
  |LCX245 LCX245|-------|                                   PE68515L         |
  |             |       | DS2401                          |--------|  SP232  |
  |PQ30RV21     |XC2S100|           XC9572XL              |NATIONAL|  25MHz  |
  |             |       |                                 |DP83815 |   93LC46|
  |             |-------|                                 |        |         |
  |          74LS245 74LS245                              |--------|        L|
  |PQ30RV21            74LS245 74LS245                                      L|
  |         IDE44   HDD_LED          LCX245 LCX245 LCX245           DIPSW(8)L|
  |---------------------------------|   LCX245  LCX245                      L|
                                    |                                       L|
                                    |                              74LS273  L|
                                    |                                       L|
                                    |   48LC4M16  |------------|            L|
                                    |             |TOSHIBA     |             |
                                    |             |TMPR3927CF  |             |
                                    |             |            |   74LS245   |
                                    |             |            |             |
                                    |             |            |             |
                                    |   48LC4M16  |------------|             |
                                    |                                        |
                                    |                8.28MHz              CN2|
                                    |                                        |
                                    |----------------------------------------|
  Notes: (all IC's shown)
        TMPR3927 - Toshiba TMPR3927CF Risc Microprocessor (QFP240)
        FLASH    - Fujitsu 29F400TC Flash ROM (TSOP48)
        IDE44    - IDE44 44-pin laptop type HDD connector. The Hard Drive connected is a
                   2.5" Fujitsu MHR2010AT 10GB HDD with Konami sticker C07JAA03
        48LC4M16 - Micron Technology 48LC4M16 4M x16-bit SDRAM (TSSOP54)
        XC9572XL - XILINX XC9572XL In-system Programmable CPLD stamped 'UC07A1' (TQFP100)
        XC2S100  - XILINX XC2S100 SPARTAN-II 2.5V FPGA (TQFP144)
        DS2401   - MAXIM Dallas DS2401 Silicon Serial Number (SOIC6)
        93LC46   - 128 bytes x8-bit EEPROM (SOIC8)
        MB3793   - Fujitsu MB3793 Power-Voltage Monitoring IC with Watchdog Timer (SOIC8)
        PE68515L - Pulse PE-68515L 10/100 Base-T Single Port Transformer Module
        DP83815  - National Semiconductor DP83815 10/100 Mb/s Integrated PCI Ethernet Media
                   Access Controller and Physical Layer (TQFP144)
        SP232    - Sipex Corporation SP232 Enhanced RS-232 Line Drivers/Receiver (SOIC16)
        RJ45     - RJ45 network connector
        DIP40    - Empty DIP40 socket
        CN1      - Custom multi-pin connector for special cable. The other end of the
                   cable has a PCMCIA card which plugs into the PCMCIA slot on a
                   System 573 main board
        CN2      - 6-pin power input connector
        CN3      - 4-pin connector
        L        - LED

*/

k573npu_device::k573npu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, KONAMI_573_NETWORK_PCB_UNIT, "Konami 573 Network PCB Unit", tag, owner, clock, "k573npu", __FILE__)
{
}

void k573npu_device::device_start()
{
}

ROM_START( k573npu )
	ROM_REGION( 0x080000, "tmpr3927", 0 )
	ROM_LOAD( "29f400.24e",   0x000000, 0x080000, CRC(8dcf294b) SHA1(efac79e18db22c30886463ec1bc448187da7a95a) )
ROM_END

const rom_entry *k573npu_device::device_rom_region() const
{
	return ROM_NAME( k573npu );
}

const device_type KONAMI_573_NETWORK_PCB_UNIT = &device_creator<k573npu_device>;
