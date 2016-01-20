// license:BSD-3-Clause
// copyright-holders:
/******************************************************************************


Integral Personal Computer (HP9807A)
Hewlett-Packard, 1985

This is a portable mains-powered UNIX workstation computer system produced by Hewlett-Packard and launched in 1985
Basic hardware specs are....
- 68000 CPU at 7.96MHz
- 9" amber electro-luminescent display with a resolution of 255*512 pixels or up to 85 characters x 31 lines (default=80*24) with
  dedicated 32Kb display memory
- Internal 3.5" floppy disk drive with the following specification....
  Encoding: Double Density HP MFM Format
  Rotational speed: 600 RPM
  Transfer speed: 62.5kB/second
  Capacity: 709Kb (709632 bytes)
  Bytes per sector: 512
  Sectors per track: 9
  Total tracks per surface: 80, Available: 77, Spare: 2, Wear: 1
  Surfaces: 2
  Interleave: 1
- HP ThinkJet ink-jet printer integrated into the top of the case. This is a modified HP2225B Thinkjet printer
- 90-key detachable keyboard
- ROM: up to 512Kb standard and an additional 512Kb of option ROM
- RAM: 512Kb, arranged as 256Kbx16. RAM is expandable externally to 7Mb
- Real Time Clock
- Speaker
- External I/O bus with two I/O ports for interfaces and memory modules. Expandable to maximum 10 ports with two 5-port Bus Expander Modules
- HP-IB (IEEE-488) bus
- Runs the HP-UX Operating System III or System V (in ROM)


PCB Layouts
===========

CPU/Memory Board (LOGIC A PCA)
----------------

HP Part# 00095-60953
                     |---------------------------------------------------------------------------------|
                     |  15.92MHz     J1        J2                                                      |
                     |LS74                                                                             |
                     |     |---|                                                                       |
                     |     |T  |                                                                       |
                     |     |M  |                                                                       |
                     |     |S  |                                                                       |
                     |     |4  |         |-------------------------|                                   |
|--------------------|     |5  |         |          68000          |                                   |
|                          |0  |         |                         |                                   |
| MB81256   MB81256        |0  |         |-------------------------|                                   |
|                          |A  |                                                                       |
| MB81256   MB81256        |---|                                                                       |
|                      |----------------J3-----------------|                                           |
| MB81256   MB81256    |                                   |                                           |
|                      |                                   |                                           |
| MB81256   MB81256    |                                   |                                           |
|                      |                                   |                                           |
| MB81256   MB81256    |                                   |                                           |
|                      |                                   |                                           |
| MB81256   MB81256    |                                   |                                           |
|                      |----------------J4-----------------|                                           |
| MB81256   MB81256                                                                                    |
|                                                                                U58            U60    |
| MB81256   MB81256                                                                      555           |
|                                                                                                      |
|                                                                                                      |
|                                                                                                      |
|      J5                                                           J6                         J7      |
|------------------------------------------------------------------------------------------------------|
Notes:
        68000 - 68000 CPU at U7. Clock input 7.96MHz [15.92/2] (DIP64)
         LS74 - 74LS74 at U2 provides system clock dividers /4 /2
      MB81256 - Fujitsu MB81256 256Kx1 DRAM. Total RAM 512Kx8/256Kx16. HP part# 1818-3308. U20-U35 (DIP16)
      TMS4500 - Texas Instruments TMS4500A DRAM Controller at U4. Clock input 3.98MHz [15.92/4] (DIP40)
          U58 - HP-HIL Keyboard Interface 'Cerberus'. Clock input on pin 24 is unknown (DIP24)
          U60 - Unknown IC used as an interrupt encoder. Possibly a logic chip? (DIP20)
          555 - 555 Timer
        J1/J2 - Connectors joining to LOGIC B PCA (logic A to logic B bus)
        J3/J4 - Connectors for ROM board
           J5 - Power input connector from power supply PCB
           J6 - 64-pin external I/O bus. This is connected to the I/O backplane PCB mounted at the rear of the case using a ribbon cable
           J7 - 10 pin connector. Labelled on schematics as LOOP0 In/Out and LOOP1 In/Out. Connected to 'Cerberus' IC. Possibly for more input devices?


Interface & Control Board (LOGIC B PCA)
-------------------------

HP Part# 00095-60952
00095-60107 BD REV A
00095-60149 ASSY LOGIC B
|---------------------------------------------------------------------------------|
|SPEAKER                        NS58167A               J11       J10              |
|            LM358   32.768kHz                                                    |
|                                                                                 |
|                                                      16Kx4  16Kx4  16Kx4  16Kx4 |
|                 ROM                                                             |
|                                                                                 |
|   BT1                       HP-IL(2)       GPU                                  |
|                                                                                 |--------------------|
|                 1Kb                                                                                  |
|  COP452                                                                                              |
|                                                                                                      |
|                                                                                                      |
|                                                                              WD2797                  |
| LM393          HP-IL(1)                                                                              |
|                                                                                                      |
|                                           LS193                                                      |
|                                                                                                      |
|                                                                                                      |
|                                                                                                      |
|                                                  LS74    24MHz      LS161                            |
|                                                                                                    J2|
|                                                                                                      |
|                                                            TMS9914                                   |
|                                                                                                      |
|                                                                                                      |
|                                                              75162   75160                           |
|                                                                                                      |
|  J14  J6     J5     J4     J3                  J7             J1         J8        J9        J12     |
|------------------------------------------------------------------------------------------------------|
Notes:
         GPU - GPU (Graphics Processor) at U1. Clock input 3MHz [24/8]. VSync on pin 45, HSync on pin 46 (DIP48)
       16Kx4 - 16Kx4 DRAM, organised as 32Kx8bit/16Kx16bit at U3, U4, U5 & U6. Chip type unknown, likely Fujitsu MB81416 (DIP18)
      WD2797 - Western Digital WD2797 Floppy Disk Controller at U18. Clock input 2MHz [24/2/3/2] (DIP40)
    HP-IL(1) - HP-IL 1LJ7-0015 'Saturn' Thinkjet Printer Controller IC at U28. Clock input 3MHz on pin 9 [24/8] (DIP48)
    HP-IL(2) - HP-IL Interface IC at U31. Clock input 2MHz on pin 22 [24/2/3/2] (DIP28)
         1Kb - 1Kb RAM at U27 for printer buffer (DIP28, type unknown, very old, with only DATA0,1,2,3, C/D and DIN,DOUT)
         ROM - 16Kb (2Kx8?) Some kind of very early DIP28 PROM/ROM? Same pinout as 1Kb RAM above. Holds the character font table for the printer
               Four versions of this ROM exist, one each for Japan/Arabic/Hebrew and one for all other regions
    NS58167A - National Semiconductor NS58167A Clock Controller RTC at U44. Clock input 32.768kHz (DIP24)
       LM358 - National Semiconductor LM358 Operational Amplifier at U40 (DIP8)
       LM393 - Texas Instruments LM393 Dual Comparator at U34 (DIP8)
         BT1 - 3v lithium battery
      COP452 - National Semiconductor COP452 Speaker Controller at U39. Clock input 2MHz [24/2/3/2]. This IC provides programmable square wave output from 100Hz to 5000Hz (DIP14)
     TMS9914 - Texas Instruments TMS9914 General Purpose Interface Bus Adapter at U41. Clock input 4MHz [24/2/3] (DIP40)
       75160 - National Semiconductor DS75160A IEEE-488 General Purpose Interface Bus Transceiver at U42 (DIP20)
       75162 - National Semiconductor DS75162A IEEE-488 General Purpose Interface Bus Transceiver at U43 (DIP22)
       LS193 - 74LS193 at U9 provides the main system clock dividers /8 /4 /2 (24MHz -> 12MHz, 6MHz, 3MHz). This is also the source of the 6MHz video clock on connector J1
       LS161 - 74LS161 at U46. Takes 12MHz clock input on pin 2 and outputs 4MHz on pin 13 (i.e. /3). This is the clock source for the TMS9914
        LS74 - LS74 at U16. Takes output of 4MHz from LS161 at U46 and outputs 2MHz on pin 5 (i.e. /2). This is the source of the 2MHz clocks
          J1 - Connector joining to video display assembly
          J2 - Connector joining to floppy drive
          J3 - Connector joining to printer front switch panel assembly (ADV/CONT/FF/ATTN)
          J4 - Connector joining to printer out of paper switch connected to ATTN
          J5 - Connector joining to printer carriage motor
          J6 - Connector joining to printer mechanism assembly including paper motor
          J7 - Connector joining to printer printhead
          J8 - Connector joining to HO?E switch (? ~ M, text unreadable on schems)
          J9 - Connector for ?
     J10/J11 - Connectors joining to LOGIC A PCA (logic A to logic B bus)
         J12 - Power input connector from power supply PCB
         J14 - Fan connector


ROM board (Operating System ROM PCA. Assembly# HP82991A or HP82995A)
---------

|----------------J3-----------------|
|                                   |
|                                   |
|                                   |
|J1  U1      U2      U3     U4    J2|
|                                   |
|                                   |
|                                   |
|----------------J4-----------------|
Notes:
      J1/J2 - 20 pin connector joining to 'Option ROM PCA'
      J3/J4 - 20 pin connector joining to 'LOGIC A PCA'
      U1-U4 - 28 pin EPROM/MASKROM 0L/1L/0H/1H (note 1Mbit: 128Kx8 28 pin)


ROM board (Option ROM PCA)
---------
Note this PCB plugs in upside-down on top of the Operating System ROM PCB

|----------------J4-----------------|
|                                   |
|                                   |
|                                   |
|J1  U1      U2      U3     U4    J2|
|                                   |
|                                   |
|                                   |
|----------------J3-----------------|
Notes:
      J1/J2 - 20 pin connector joining to 'Operating System ROM PCA'
      J3/J4 - 20 pin connector joining to 'LOGIC A PCA'
      U1-U4 - 28 pin EPROM/MASKROM 0L/1L/0H/1H (note 1Mbit: 128Kx8 28 pin)


Physical Memory Map
===================
000000-07FFFF - Internal ROM (Operating System PCA 512Kb)
080000-0FFFFF - Internal ROM (Option ROM PCA 512Kb)
100000-4FFFFF - External ROM modules (up to 4Mb)
500000-5FFFFF - Reserved 1Mb
600000-6FFFFF - Internal I/O 1Mb

                Address        Port  Use
                -------------------------
                600000-60FFFF   0    MMU
                610000-61FFFF   1    Disk Drive
                620000-62FFFF   2    Display
                630000-63FFFF   3    HP-IB
                640000-64FFFF   4    RTC
                650000-65FFFF   5    Printer
                660000-66FFFF   6    Keyboard
                670000-67FFFF   7    Speaker
                680000-68FFFF   8    Reserved
                690000-69FFFF   9    Reserved
                6A0000-6AFFFF   10   Reserved
                6B0000-6BFFFF   11   Reserved
                6C0000-6CFFFF   12   Reserved
                6D0000-6DFFFF   13   Reserved
                6E0000-6EFFFF   14   Reserved
                6F0000-6FFFFF   15   Reserved

700000-7FFFFF - External I/O 1Mb

                Address        Port  Use
                -------------------------
                700000-70FFFF   16   Mainframe Port A
                710000-71FFFF   17   Mainframe Port B
                720000-72FFFF   18   Bus Expander Port A1
                730000-73FFFF   19   Bus Expander Port A2
                740000-74FFFF   20   Bus Expander Port A3
                750000-75FFFF   21   Bus Expander Port A4
                760000-76FFFF   22   Bus Expander Port A5
                770000-77FFFF   23   Reserved
                780000-78FFFF   24   Reserved
                790000-79FFFF   25   Reserved
                7A0000-7AFFFF   26   Bus Expander Port B1
                7B0000-7BFFFF   27   Bus Expander Port B2
                7C0000-7CFFFF   28   Bus Expander Port B3
                7D0000-7DFFFF   29   Bus Expander Port B4
                7E0000-7EFFFF   30   Bus Expander Port B5
                7F0000-7FFFFF   31   Reserved

800000-EFFFFF - External RAM modules (up to 7Mb)
F00000-F7FFFF - Internal RAM 512Kb
F80000-FFFFFF - Reserved 512Kb

Access to 800000-FFFFFF can be remapped by the MMU registers.


Interrupts
----------
High Priority 7 - Soft reset from keyboard (NMI)
     /\       6 - RTC or NBIR3 (external I/O)
     ||       5 - Disc Drive or NBIR2 (external I/O)
     ||       4 - GPU or NBIR1 (external I/O)
     ||       3 - HP-IB, printer or NBIR0 (external I/O)
     \/       2 - HP-HIL devices (keyboard/mouse)
Low Priority  1 - RTC

Note external interrupt lines NBIR0 to NBIR3 can be asserted by an interface connected to the external I/O port


******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"


class hp_ipc_state : public driver_device
{
public:
	hp_ipc_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_WRITE16_MEMBER(mmu_w);
	DECLARE_READ16_MEMBER(ram_r);
	DECLARE_WRITE16_MEMBER(ram_w);

private:
	required_device<m68000_device> m_maincpu;

	UINT32 m_mmu[4];
	UINT16 m_internal_ram[0x40000];

	inline UINT32 get_ram_address(offs_t offset) { return (m_mmu[(m_maincpu->get_fc() >> 1) & 3] + offset) & 0x3FFFFF; }
};


static ADDRESS_MAP_START(hp_ipc_mem, AS_PROGRAM, 16, hp_ipc_state)
	AM_RANGE(0x000000, 0x07FFFF) AM_ROM
	AM_RANGE(0x600000, 0x60FFFF) AM_WRITE(mmu_w)
	AM_RANGE(0x800000, 0xFFFFFF) AM_READWRITE(ram_r, ram_w)
ADDRESS_MAP_END


static INPUT_PORTS_START(hp_ipc)
INPUT_PORTS_END


WRITE16_MEMBER(hp_ipc_state::mmu_w)
{
	logerror("mmu_w: offset = %08x, data = %04x, register = %d, data_to_add = %08x\n", offset, data, offset & 3, (data & 0xFFF) << 10);
	m_mmu[offset & 3] = (data & 0xFFF) << 10;
}


READ16_MEMBER(hp_ipc_state::ram_r)
{
	UINT32 ram_address = get_ram_address(offset);

	//logerror("RAM read, offset = %08x, ram address = %08X\n", offset, ram_address);

	if (ram_address < 0x380000)
	{
		// External RAM modules
	}
	else if (ram_address < 0x3c0000)
	{
		// Internal RAM
		return m_internal_ram[offset & 0x3ffff];
	}
	return 0xffff;
}


WRITE16_MEMBER(hp_ipc_state::ram_w)
{
	UINT32 ram_address = get_ram_address(offset);

	//logerror("RAM write, offset = %08x, ram address = %08X, data = %04x\n", offset, ram_address, data);

	if (ram_address < 0x380000)
	{
		// External RAM modules
	}
	else if (ram_address < 0x3c0000)
	{
		// Internal RAM
		m_internal_ram[offset & 0x3ffff] = data;
	}
}


static MACHINE_CONFIG_START(hp_ipc, hp_ipc_state)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 15920000 / 2)
	MCFG_CPU_PROGRAM_MAP(hp_ipc_mem)
MACHINE_CONFIG_END


ROM_START(hp_ipc)
	ROM_REGION(0x100000, "maincpu" , 0)
	ROM_LOAD("hp ipc os 82991A.bin", 0x00000, 0x80000, BAD_DUMP CRC(df45a37b) SHA1(476af9923bca0d2d0f40aeb81be5145ca76fddf5)) // Should be spread across 4 x 128K ROMs
ROM_END


COMP(1985, hp_ipc, 0, 0, hp_ipc, hp_ipc, driver_device, 0, "HP", "Integral Personal Computer", MACHINE_IS_SKELETON)
