// license:BSD-3-Clause
// copyright-holders:

/**********************************************************************************************************************

Skeleton driver for Cisco CSS11501S-K9 Content Services Switch with SSL.

The appliance has eight 10/100 Ethernet ports, one 10Base-T port for management, one RJ45 serial port for console, two
PCMCIA slots for storage and one Gigabit Ethernet port (GBIC).

Hardware components:

Main board.
 -Sipex SP3243ECA Intelligent RS-232 Transceiver.
 -Crystal CrystalLAN CS8900A Ethernet ISA LAN Controller.
 -Exar ST16C1551 UART.
 -Microchip PIC16LF877PT next to a 10 MHz crystal (U35).
 -Xilinx Spartan XC2S50 (U23).
 -Atmel 24C02N SEEPROM (U11) next to the Xilinx Spartan XC2S50 at U23.
 -Two PCMCIA slots for Microdrive or Flash Drive.
 -Intel SPD6722QCCE PC-Card/PCMCIA controller.
 -Two 24C04R6 SEEPROMs near the two PCMCIA slots (U25 and U32).
 -Sipex 3223ECY RS-232 transceiver.
 -ST M4T32-BR12SH6 Timekeeper.
 -Xilinx Spartan XC2S50 (another one, U33).
 -Another 24C02N (U28), near the Xilinx Spartan XC2S50 at U33.
 -Vitesse VSC2102-08UQ Network Processor / Intelligent Packet Processor, near a 25 MHz crystal.
 -Intel LXT9785EHC 8-port Fast Ethernet PHY Transceiver near a 50 MHz crystal.
 -PMC RM7000A 400T 64-Bit MIPS RISC Microprocessor with Integrated L2 cache.
 -AMD AM29LV3200B Flash Memory.
 -PMC PM2329-BC.
 -Vitesse VSC2708-00UR.
 -AMCC S2068TB.
 -Cypress CY7C1360B-166AC 9-Mbit (256K x 36) Pipelined SRAM.
 -Samsung K7N403601B-QC13 128Kx36 Pipelined NtRAM.
 -SORIMM slot with a 256MB RAM module (Samsung MS18R1628EH0-CM8CI).
 -Cypress CY7C1360B-166AC 9-Mbit (256K x 36) Pipelines SRAM (another one).
 -Intel SPD6722QCCE PC-Card/PCMCIA controller.
 -Xilinx XC18V02 next to the first Xilinx Spartan XC2S50 (U23).

Sub-board (SSL acceleration).
 -Broadcom BCM5821A1KTB Super E-Commerce Processor (high-performance public-key processor) next to a 100 MHz crystal.
 -Martel GT-64120A-BN-3 (System Controller for 7000 CPU) next to another 100 MHz crystal.
 -PMC RM7000A 400T 64-Bit MIPS RISC Microprocessor with Integrated L2 cache.
 -AMD AM29LV1600B Flash Memory.
 -Altera Max EPM3128ATC100-5
 -Broadcom BCM5700C2KPB PCI-X 10/100/1000 Base-T controller, near a 125 MHz crystal.
 -Vitesse VSC2102-08UQ Network Processor / Intelligent Packet Processor, near a 25 MHz crystal.
 -IDT 71V546S133PF 128K x 36, 3.3V Synchronous SRAM with ZBT Feature, Burst Counter and Pipelined Outputs.
 -24C04R SEEPROM between the BCM5700C2KPB, the VSC2102-08UQ, and the 71V546S133PF.
 -Microchip PIC16LF872 next to a 10 MHz crystal.
 -Xilinx Spartan XC2S50.
 -Xilinx 18V01JC In-System Programmable Configuration PROM next to the Xilinx Spartan XC2S50.
 -SORIMM slot with a 64MB RAM module (Samsung MS18R1622EH0-CM8).
 -SODIMM slot with a 512MB RAM module (Cisco CIS00-21160-111CD).

**********************************************************************************************************************/

#include "emu.h"
#include "cpu/mips/mips3.h"


namespace {

class css11501sk9_state : public driver_device
{
public:
	css11501sk9_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sslcpu(*this, "sslcpu")
	{ }

	void css11501sk9(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_sslcpu;
};


// Input ports
static INPUT_PORTS_START( css11501sk9 )
INPUT_PORTS_END


void css11501sk9_state::machine_reset()
{
}

void css11501sk9_state::machine_start()
{
}

void css11501sk9_state::css11501sk9(machine_config &config)
{
	// basic machine hardware
	RM7000BE(config, m_maincpu, 400'000'000); // Main CPU, PMC RM7000A 400T
	RM7000BE(config, m_sslcpu, 400'000'000); // SSL CPU, PMC RM7000A 400T
}


ROM_START( css11501sk9 )
	ROM_REGION(0x410000, "maincpu", 0) // Main
	ROM_LOAD( "css11501s-k9_73-8174-04_17-7009-03_cs-a393_0514_hm_am29lv320db.u46",      0x000000, 0x410000, CRC(fdc00a8f) SHA1(cc89c39462e783d874abebbed2a80bd9ec095047) )

	ROM_REGION(0x200000, "sslcpu", 0) // SSL
	ROM_LOAD( "css11501s-k9_73-6917-06_17-6860-03_cs-7edd_0525_am29lv160db.u36",         0x000000, 0x200000, CRC(6dfe6620) SHA1(06cc9015907c74284a6b85393c0024729146a19b) )

	ROM_REGION(0x001eb3, "plds", 0)
	ROM_LOAD( "css11501s-k9_73-6917-06_17-6858-01_172f5a_2305_i4ps_epm3128atc100-5.u11", 0x000000, 0x001eb3, CRC(a094a0d3) SHA1(a4897096fd7b26b3a3e3c928fa8d1cfd2a2f5641) ) // Altera Max EPM3128ATC100-5 on SSL sub-board

	ROM_REGION(0x040000, "fpga", 0)
	ROM_LOAD( "css11501s-k9_73-6917-06_17-6859-02_cs-32cb_0519_xilinx_18v01jc.u2",       0x000000, 0x020000, CRC(bd69fa75) SHA1(9ff93c1f202ad1c178996e7ffd2afd644afd3fe6) ) // Xilinx 18V01JC next to the Xilinx Spartan XC2S50, on SSL sub-board
	ROM_LOAD( "css11501s-k9_73-8174-04_17-7008-02_cs-3be7_0524_xc18v02cg44.u43",         0x000000, 0x040000, CRC(a258b636) SHA1(f1cd8409362572cc45fa077179d98a1bb0df383f) ) // Xilinx XC18V02 next to the Xilinx Spartan XC2S50 at U23, on main PCB

	ROM_REGION(0x000200, "seeprom", 0)
	ROM_LOAD( "css11501s-k9_73-8174-04_24c02n.u11",                                      0x000000, 0x000100, CRC(bbe4ba95) SHA1(a56ebd9a98a51d63178da7c93c2b9e557e987a5c) ) // 24C02N SEEPROM near the Xilinx Spartan XC2S50 at U23, on main PCB
	ROM_LOAD( "css11501s-k9_73-8174-04_24c02n.u28",                                      0x000000, 0x000100, CRC(a10ac3ca) SHA1(c68610c51106d2351570c1c77ab1cf1fb210ffb1) ) // 24C02N SEEPROM near the Xilinx Spartan XC2S50 at U33, on main PCB
	ROM_LOAD( "css11501s-k9_73-8174-04_24c04r6.u25",                                     0x000000, 0x000200, CRC(cfe0b9c1) SHA1(2af0645dc5aaab26c0330ef97b97c4958f3ff173) ) // 24C04R6 SEEPROM near the PCMCIA slots, on main PCB
	ROM_LOAD( "css11501s-k9_73-8174-04_24c04r6.u32",                                     0x000000, 0x000200, CRC(d2b2c2d3) SHA1(ea2ec6863749e1020a79710b36d1ac42528c7855) ) // 24C04R6 SEEPROM near the PCMCIA slots, on main PCB
	ROM_LOAD( "css11501s-k9_73-6917-06_24c04r6.u33",                                     0x000000, 0x000200, CRC(bd7bc39f) SHA1(9d0ac37bb3ec8c95990fd37a962a17a95ce97aa0) ) // 24C04R SEEPROM between the BCM5700C2KPB, the VSC2102-08UQ, and the 71V546S133PF, on SSL sub-board
	ROM_LOAD( "css11501s-k9_73-6917-06_24c02n.u8",                                       0x000000, 0x000100, CRC(dcbe3083) SHA1(81934196733a8878cc19ced4f1791dfaa4da494a) ) // 24C02N SEEPROM between the BCM5700C2KPB, the 71V546S133PF, and the XC2S50, on SSL sub-board

	ROM_REGION(0x004400, "pic", 0)
	// CONFIG = 3eh, ID = ff3fff3fff3fff3fh
	ROM_LOAD( "css11501s-k9_73-8174-04_17-6486-03_2605_27af12_pic16lf877pt_user.u35",    0x000000, 0x004000, CRC(22b03fe9) SHA1(984c14fb013763aa4fb981888480d19a6058843b) ) // PIC16LF877, on main PCB
	ROM_LOAD( "css11501s-k9_73-8174-04_17-6486-03_2605_27af12_pic16lf877pt_data.u35",    0x000000, 0x000200, CRC(14bfe63a) SHA1(ebe833da793930303ac57c873a97f8383494a939) ) // PIC16LF877, on main PCB

	// CONFIG = 3eh, ID = 7f007f007f007f00h
	ROM_LOAD( "css11501s-k9_73-8174-04_17-6494-02_1605_92817_pic16lf872_user.u7",        0x000000, 0x001000, CRC(6deebf3a) SHA1(c8f030657b7959c6d3aa5b1583786eb84ed199c1) ) // PIC16LF872, on SSL sub-board
	ROM_LOAD( "css11501s-k9_73-8174-04_17-6494-02_1605_92817_pic16lf872_data.u7",        0x000000, 0x000080, CRC(903500ff) SHA1(d33277afda835773cfcc9bc137690cbe79943f0f) ) // PIC16LF872, on SSL sub-board
ROM_END

} // Anonymous namespace

//    YEAR  NAME         PARENT  COMPAT  MACHINE      INPUT        CLASS              INIT        COMPANY  FULLNAME        FLAGS
SYST( 2002, css11501sk9, 0,      0,      css11501sk9, css11501sk9, css11501sk9_state, empty_init, "Cisco", "CSS11501S-K9", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
