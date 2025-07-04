// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/******************************************************************************

Integral Personal Computer (HP9807A)
Hewlett-Packard, 1985

Driver to-do list
=================

- keyboard: NMI generation
- RTC chip: proper month, day (possibly a different chip, 82167)
- HP-IL printer

QA
+ diagnstc.td0: display test
- diagnstc.td0: complete keyboard test [second connector not implemented]
+ diagnstc.td0: speaker test
- diagnstc.td0: printer test
+ diagnstc.td0: auto: floppy disc test
+ diagnstc.td0: auto: ram test
- diagnstc.td0: auto: rtc test [cannot execute]
+ diagnstc.td0: auto: short keyboard test

maybe
- drive AP line of MLC from a timer
- RTC standby interrupt?
- what does _desktop do except setting 640x400 mode?
- non-HLE keyboard and mouse? (need dumps of COP4xx)

slot devices
- 82915A -- 300/1200 bps modem; http://www.hpmuseum.net/display_item.php?hw=920
- 82919A -- serial; http://www.hpmuseum.net/display_item.php?hw=445
- 82920A -- current loop; http://www.hpmuseum.net/display_item.php?hw=975
- 82922A -- BCD interface; http://www.hpmuseum.net/display_item.php?hw=921
- 82923A -- GPIO; http://www.hpmuseum.net/display_item.php?hw=976
- 82924A -- HP-IL; http://www.hpmuseum.net/display_item.php?hw=922
- 82968A -- up to 256 KB of ROM on top of operating system PCA
- 82971A -- up to 1 MB of EPROM or 2 MB or masked ROM
- 82998A -- HP-IB; http://www.hpmuseum.net/display_item.php?hw=933
- 98071A -- 640x400 composite Video + Serial; http://www.hpmuseum.net/display_item.php?hw=935


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
          U58 - 1RD2-6001, HP-HIL Master Link Controller 'Cerberus'. Clock input on pin 24 is unknown (DIP24)
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
         GPU - 1LL3-0005 GPU (Graphics Processor) at U1. Clock input 3MHz [24/8]. VSync on pin 45, HSync on pin 46 (DIP48)
       16Kx4 - 16Kx4 DRAM, organised as 32Kx8bit/16Kx16bit at U3, U4, U5 & U6. Chip type unknown, likely Fujitsu MB81416 (DIP18)
      WD2797 - Western Digital WD2797 Floppy Disk Controller at U18. Clock input 2MHz [24/2/3/2] (DIP40)
    HP-IL(1) - HP-IL 1LJ7-0015 'Saturn' Thinkjet Printer Controller IC at U28. Clock input 3MHz on pin 9 [24/8] (DIP48)
    HP-IL(2) - HP-IL 1LB3 Interface IC at U31. Clock input 2MHz on pin 22 [24/2/3/2] (DIP28)
         1Kb - 1Kb RAM at U27 for printer buffer (DIP28, type unknown, very old, with only DATA0,1,2,3, C/D and DIN,DOUT)
         ROM - 16Kb (32Kx4) Some kind of very early DIP28 PROM/ROM? Same pinout as 1Kb RAM above. Holds the character font table for the printer
               Four versions of this ROM exist, one each for Japan/Arabic/Hebrew and one for all other regions
    NS58167A - National Semiconductor NS58167A Clock Controller RTC at U44. Clock input 32.768kHz (DIP24)
               (Tony Duell's schematics show a 82167 instead)
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
      U1-U4 - 28 pin EPROM/mask ROM 0L/1L/0H/1H (note 1Mbit: 128Kx8 28 pin)


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
      U1-U4 - 28 pin EPROM/mask ROM 0L/1L/0H/1H (note 1Mbit: 128Kx8 28 pin)


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


Interrupts (all autovectored)
----------
High Priority 7 - Soft reset from keyboard (NMI)
     /\       6 - RTC or NBIR3 (external I/O)
     ||       5 - Disc Drive or NBIR2 (external I/O)
     ||       4 - GPU or NBIR1 (external I/O)
     ||       3 - HP-IB, printer or NBIR0 (external I/O)
     \/       2 - HP-HIL devices (keyboard/mouse)
Low Priority  1 - RTC

Note external interrupt lines NBIR0 to NBIR3 can be asserted by an interface connected to the external I/O port


Useful links etc.
-----------------

bitsavers://pdf/hp/integral/00095-90126_Integral_Personal_Computer_Service_Jan86.pdf

bitsavers://pdf/hp/hp-hil/45918A-90001_HP-HIL_Technical_Reference_Manual_Jan86.pdf
    HP-HIL MLC, SLC datasheets

bitsavers://pdf/sony/floppy/Sony_OA-D32_Microfloppy_Service_Nov83.pdf
    OA-D32W

http://www.hpl.hp.com/hpjournal/pdfs/IssuePDFs/1983-01.pdf
    HP-IL issue

http://www.hpl.hp.com/hpjournal/pdfs/IssuePDFs/1985-10.pdf
    IPC issue

http://www.hpl.hp.com/hpjournal/pdfs/IssuePDFs/1987-06.pdf
    HP-HIL article

http://www.hpmuseum.net/pdf/ComputerNews_1985_Jan15_37pages_OCR.pdf
    introducing the IPC

http://www.hpmuseum.net/pdf/ComputerFocus_1985_Nov_25pages_OCR.pdf
    SysV upgrade

http://www.hpmuseum.net/pdf/InformationSystemsAndManufacturingNews_81pages_Jun1-86_OCR.pdf
    EPROM/ROM modules

http://www.hpmuseum.net/pdf/HPChannels_1986_11_37pages_Nov86_OCR.pdf
    SW Eng ROM and serial option for it

http://www.coho.org/~pete/downloads/IPC/burst/Freeware/IPC_Driver_Writers_Disc/hp-ux.5.0.0
    kernel namelist

http://www.hpmuseum.net/display_item.php?hw=122
    overview, manuals, software

http://www.ambry.com/hp-computer-model/9807A.html
    replacement parts

http://www.brouhaha.com/~eric/hpcalc/chips/
    chip part numbers


Software to look for
--------------------

00095-60978 "Service ROM - Used in troubleshooting the integral PC" via ambry
00095-60925 "Service ROM" via service manual
00095-60969 "Service Diagnostic Disc" via service manual
00095-60950 "I/O Component-Level Diagnostic Disc" via serial interface service manual

00095-60006 (original System III-based HP-UX 1.0 ROM)
82995A (same bits as 82991A; the latter is an upgrade kit, former was pre-installed)
82989J Technical Basic ROM (Jan'1986)
82987A Software Engineering ROM (Nov'1986)

******************************************************************************/

#include "emu.h"
#include "hp_ipc_optrom.h"

#include "bus/hp_hil/hil_devices.h"
#include "bus/hp_hil/hp_hil.h"
#include "bus/hp_ipc_io/hp_ipc_io.h"
#include "bus/ieee488/ieee488.h"
#include "cpu/m68000/m68000.h"
#include "imagedev/floppy.h"
#include "machine/bankdev.h"
#include "machine/cop452.h"
#include "machine/input_merger.h"
#include "machine/mm58167.h"
#include "machine/ram.h"
#include "machine/tms9914.h"
#include "machine/wd_fdc.h"
#include "softlist_dev.h"
#include "sound/dac.h"
#include "video/hp1ll3.h"

#include "formats/hp_ipc_dsk.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class hp_ipc_state : public driver_device
{
public:
	hp_ipc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bankdev(*this, "bankdev")
		, m_fdc(*this, "fdc")
		, m_ram(*this, RAM_TAG)
		, m_gpu(*this , "gpu")
		, m_screen(*this, "screen")
		, m_spkr(*this , "spkr")
		, m_dac(*this , "dac")
		, m_irq3_merger(*this , "merge_irq3")
		, m_irq4_merger(*this , "merge_irq4")
		, m_irq5_merger(*this , "merge_irq5")
		, m_irq6_merger(*this , "merge_irq6")
		, m_io_slot_a(*this , "slot_a")
		, m_io_slot_b(*this , "slot_b")
		, m_rom_slots(*this , "rom%u" , 0)
	{ }

	void hp_ipc_base(machine_config &config);
	void hp_ipc(machine_config &config);
	void hp9808a(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint16_t mem_r(offs_t offset, uint16_t mem_mask);
	void mem_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t mmu_r(offs_t offset);
	void mmu_w(offs_t offset, uint16_t data);
	uint16_t ram_r(offs_t offset, uint16_t mem_mask);
	void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void spkr_w(offs_t offset, uint16_t data);

	uint8_t floppy_id_r();
	void floppy_id_w(uint8_t data);
	static void floppy_formats(format_registration &fr);

	void irq_1(int state);
	void irq_2(int state);
	void irq_3(int state);
	[[maybe_unused]] void irq_4(int state);
	void irq_5(int state);
	[[maybe_unused]] void irq_6(int state);
	void irq_7(int state);

	void hp_ipc_mem_inner_base(address_map &map) ATTR_COLD;
	void hp_ipc_mem_inner_9807a(address_map &map) ATTR_COLD;
	void hp_ipc_mem_outer(address_map &map) ATTR_COLD;

	void hp_ipc_mem_inner_9808a(address_map &map) ATTR_COLD;

	required_device<m68000_device> m_maincpu;
	required_device<address_map_bank_device> m_bankdev;
	required_device<wd2797_device> m_fdc;
	required_device<ram_device> m_ram;
	required_device<hp1ll3_device> m_gpu;
	required_device<screen_device> m_screen;
	required_device<cop452_device> m_spkr;
	required_device<dac_1bit_device> m_dac;
	required_device<input_merger_any_high_device> m_irq3_merger;
	required_device<input_merger_any_high_device> m_irq4_merger;
	required_device<input_merger_any_high_device> m_irq5_merger;
	required_device<input_merger_any_high_device> m_irq6_merger;
	required_device<hp_ipc_io_slot_device> m_io_slot_a;
	required_device<hp_ipc_io_slot_device> m_io_slot_b;
	required_device_array<hp_ipc_optrom_device , 2> m_rom_slots;

	uint32_t m_mmu[4]{}, m_lowest_ram_addr = 0;
	uint16_t *m_internal_ram = nullptr;
	int m_fc = 0;

	floppy_image_device *m_floppy = nullptr;

	inline uint32_t get_ram_address(offs_t offset)
	{
		return (m_mmu[(m_maincpu->get_fc() >> 1) & 3] + offset) & 0x3FFFFF;
	}
};

void hp_ipc_state::hp_ipc_mem_outer(address_map &map)
{
	map(0x000000, 0xFFFFFF).rw(FUNC(hp_ipc_state::mem_r), FUNC(hp_ipc_state::mem_w));
}

void hp_ipc_state::hp_ipc_mem_inner_base(address_map &map)
{
// bus error handler
	map(0x0000000, 0x1FFFFFF).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));

// user mode
	map(0x1000000, 0x17FFFFF).rw(FUNC(hp_ipc_state::ram_r), FUNC(hp_ipc_state::ram_w));
	map(0x1800000, 0x187FFFF).rom().region("maincpu", 0);
	map(0x1E00000, 0x1E0FFFF).rw(FUNC(hp_ipc_state::mmu_r), FUNC(hp_ipc_state::mmu_w));
	map(0x1E20000, 0x1E2000F).rw(m_gpu, FUNC(hp1ll3_device::read), FUNC(hp1ll3_device::write)).umask16(0x00ff);
	map(0x1E40000, 0x1E4002F).rw("rtc", FUNC(mm58167_device::read), FUNC(mm58167_device::write)).umask16(0x00ff);

// supervisor mode
	map(0x0000000, 0x007FFFF).rom().region("maincpu", 0);       // Internal ROM (operating system PCA)
	map(0x0080000, 0x00FFFFF).unmaprw();     // Internal ROM (option ROM PCA)
	map(0x0100000, 0x04FFFFF).unmaprw();     // External ROM modules
	map(0x0600000, 0x060FFFF).rw(FUNC(hp_ipc_state::mmu_r), FUNC(hp_ipc_state::mmu_w));
	map(0x0610000, 0x0610007).rw(FUNC(hp_ipc_state::floppy_id_r), FUNC(hp_ipc_state::floppy_id_w)).umask16(0x00ff);
	map(0x0610008, 0x061000F).rw(m_fdc, FUNC(wd2797_device::read), FUNC(wd2797_device::write)).umask16(0x00ff);
	map(0x0620000, 0x0620007).rw(m_gpu, FUNC(hp1ll3_device::read), FUNC(hp1ll3_device::write)).umask16(0x00ff);
	map(0x0630000, 0x063FFFF).mask(0xf).rw("hpib" , FUNC(tms9914_device::read) , FUNC(tms9914_device::write)).umask16(0x00ff);
	map(0x0640000, 0x064002F).rw("rtc", FUNC(mm58167_device::read), FUNC(mm58167_device::write)).umask16(0x00ff);
	map(0x0660000, 0x06600FF).rw("mlc", FUNC(hp_hil_mlc_device::read), FUNC(hp_hil_mlc_device::write)).umask16(0x00ff);  // 'caravan', scrn/caravan.h
	map(0x0670000, 0x067FFFF).w(FUNC(hp_ipc_state::spkr_w));    // Speaker (NatSemi COP 452)
	map(0x0670000, 0x067FFFF).nopr();   // Speaker (NatSemi COP 452)
	map(0x0680000, 0x068FFFF).noprw();       // 'SIMON (98628) fast HP-IB card' -- sys/simon.h
	map(0x0700000, 0x07FFFFF).unmaprw();     // External I/O
	map(0x0800000, 0x0FFFFFF).rw(FUNC(hp_ipc_state::ram_r), FUNC(hp_ipc_state::ram_w));
}

void hp_ipc_state::hp_ipc_mem_inner_9807a(address_map &map)
{
	hp_ipc_mem_inner_base(map);
	map(0x0650000, 0x065FFFF).noprw();       // HP-IL Printer (optional; ROM sets _desktop to 0 if not mapped) -- sys/lpint.h
}

void hp_ipc_state::hp_ipc_mem_inner_9808a(address_map &map)
{
	hp_ipc_mem_inner_base(map);
}

static INPUT_PORTS_START(hp_ipc)
INPUT_PORTS_END


uint16_t hp_ipc_state::mmu_r(offs_t offset)
{
	uint16_t data = (m_mmu[offset & 3] >> 10);

	return data;
}

void hp_ipc_state::mmu_w(offs_t offset, uint16_t data)
{
	m_mmu[offset & 3] = (data & 0xFFF) << 10;
}

uint16_t hp_ipc_state::mem_r(offs_t offset, uint16_t mem_mask)
{
	int fc = m_maincpu->get_fc() & 4;

	if (fc != m_fc)
	{
		m_fc = fc;
		m_bankdev->set_bank(m_fc ? 0 : 1);
	}

	return m_bankdev->read16(offset, mem_mask);
}

void hp_ipc_state::mem_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int fc = m_maincpu->get_fc() & 4;

	if (fc != m_fc)
	{
		m_fc = fc;
		m_bankdev->set_bank(m_fc ? 0 : 1);
	}

	m_bankdev->write16(offset, data, mem_mask);
}

void hp_ipc_state::spkr_w(offs_t offset, uint16_t data)
{
	m_spkr->cs_w(!BIT(data , 0));
	m_spkr->sk_w(BIT(data , 1));
	m_spkr->di_w(BIT(data , 2));
}

uint16_t hp_ipc_state::ram_r(offs_t offset, uint16_t mem_mask)
{
	uint32_t ram_address = get_ram_address(offset);
	uint16_t data = 0xffff;

	if (ram_address < m_lowest_ram_addr)
	{
		if (!machine().side_effects_disabled())
			m_maincpu->trigger_bus_error();
	}
	else if (ram_address < 0x3c0000)
	{
		// Internal RAM
		data = m_internal_ram[ram_address - m_lowest_ram_addr];
	}

	return data;
}

void hp_ipc_state::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint32_t ram_address = get_ram_address(offset);

	if (ram_address < m_lowest_ram_addr)
	{
		m_maincpu->trigger_bus_error();
	}
	else if (ram_address < 0x3c0000)
	{
		// Internal RAM
		COMBINE_DATA(&m_internal_ram[ram_address - m_lowest_ram_addr]);
	}
}


/*
 * bit 6 -- INTRQ
 * bit 1 -- disk changed (from drive)
 * bit 0 -- write protect (from drive)
 */
uint8_t hp_ipc_state::floppy_id_r()
{
	uint8_t data = 0;

	// TODO: fix sys controller switch
	data = (m_fdc->intrq_r() << 6) | 0x80;

	if (m_floppy)
	{
		data |= (m_fdc->intrq_r() << 6) | (m_floppy->dskchg_r() << 1) | m_floppy->wpt_r();
	}

	return data;
}

/*
 * bit 7 -- 1: motor on (via inverter to drive's /MTorOn)
 * bit 3 -- 1: head 0, 0: head 1 (via inverter to drive's /SSO)
 * bit 1 -- 1: drive select (via inverter to drive's /DRIVE SEL 1)
 * bit 0 -- 1: reset disc_changed (via inverter to drive's /DSKRST)
 */
void hp_ipc_state::floppy_id_w(uint8_t data)
{
	floppy_image_device *floppy0 = m_fdc->subdevice<floppy_connector>("0")->get_device();

	if (!BIT(data, 1))
	{
		if (m_floppy == nullptr)
		{
			m_floppy = floppy0;
			m_fdc->set_floppy(m_floppy);
		}
	}
	else
	{
		m_floppy = nullptr;
	}

	if (m_floppy)
	{
		m_floppy->ss_w(BIT(data, 3));
		m_floppy->mon_w(!BIT(data, 7));
		if (BIT(data, 0))
			m_floppy->dskchg_w(0);
	}
	else
	{
		floppy0->mon_w(1);
	}
}


void hp_ipc_state::irq_1(int state)
{
	m_maincpu->set_input_line(M68K_IRQ_1, state);
}

void hp_ipc_state::irq_2(int state)
{
	m_maincpu->set_input_line(M68K_IRQ_2, state);
}

void hp_ipc_state::irq_3(int state)
{
	m_irq3_merger->in_w<0>(state);
}

void hp_ipc_state::irq_4(int state)
{
	m_irq4_merger->in_w<0>(state);
}

void hp_ipc_state::irq_5(int state)
{
	m_irq5_merger->in_w<0>(state);
}

void hp_ipc_state::irq_6(int state)
{
	m_irq6_merger->in_w<0>(state);
}

void hp_ipc_state::irq_7(int state)
{
	m_maincpu->set_input_line(M68K_IRQ_7, state);
}


void hp_ipc_state::machine_start()
{
	m_bankdev->set_bank(1);

	m_lowest_ram_addr = 0x3c0000 - (m_ram->size() >> 1);
	m_internal_ram = (uint16_t *)m_ram->pointer();

	m_io_slot_a->install_read_write_handlers(m_bankdev->space());
	m_io_slot_b->install_read_write_handlers(m_bankdev->space());
}

void hp_ipc_state::machine_reset()
{
	m_floppy = nullptr;
	m_spkr->cs_w(1);
	m_spkr->sk_w(0);
	m_spkr->di_w(0);

	// Load optional ROMs (if any)
	// But first unload anything in opt ROM range
	m_bankdev->space().unmap_read(0x100000 , 0x4fffff);
	m_bankdev->space().unmap_read(0x100000 + 0x1800000, 0x4fffff + 0x1800000);

	for (auto& slot : m_rom_slots) {
		slot->install_read_handler(m_bankdev->space());
	}
}


void hp_ipc_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_HP_IPC_FORMAT);
}

static void hp_ipc_floppies(device_slot_interface &device)
{
	device.option_add("35dd", SONY_OA_D32W);
}

/*
 *  IRQ levels (page 5-4)
 *
 *  7   Soft reset from keyboard, non-maskable
 *  6   Real-time clock or NBIR3 (ext. I/O)
 *  5   Disc Drive or NBIR2
 *  4   GPU or NBIR1
 *  3   HP-IB, printer, or NBIR0
 *  2   HP-HIL devices (keyboard, mouse)
 *  1   Real-time clock
 */
void hp_ipc_state::hp_ipc_base(machine_config &config)
{
	M68000(config, m_maincpu, 15.92_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp_ipc_state::hp_ipc_mem_outer);

	HP1LL3(config , m_gpu , 24_MHz_XTAL / 8).set_screen("screen");

	// FIXME: actual clock is 1MHz; remove this workaround (and change 2000 to 1000 in hp_ipc_dsk.cpp)
	// when floppy code correctly handles 600 rpm drives.
	WD2797(config, m_fdc, 2_MHz_XTAL);
	m_fdc->intrq_wr_callback().set(FUNC(hp_ipc_state::irq_5));
	FLOPPY_CONNECTOR(config, "fdc:0", hp_ipc_floppies, "35dd", hp_ipc_state::floppy_formats);

	SOFTWARE_LIST(config, "flop_list").set_original("hp_ipc");
	SOFTWARE_LIST(config, "rom_list").set_original("hp_ipc_rom");

	mm58167_device &rtc(MM58167(config, "rtc", 32.768_kHz_XTAL));
	rtc.irq().set(FUNC(hp_ipc_state::irq_1));
//  rtc.standby_irq().set(FUNC(hp_ipc_state::irq_6));

	hp_hil_mlc_device &mlc(HP_HIL_MLC(config, "mlc", XTAL(15'920'000)/2));
	mlc.int_callback().set(FUNC(hp_ipc_state::irq_2));
	mlc.nmi_callback().set(FUNC(hp_ipc_state::irq_7));

	HP_HIL_SLOT(config, "hil1", "mlc", hp_hil_devices, "hp_ipc_kbd");
	HP_HIL_SLOT(config, "hil2", "mlc", hp_hil_devices, "hp_46060b");

	tms9914_device &hpib(TMS9914(config, "hpib", 4_MHz_XTAL));
	hpib.int_write_cb().set(FUNC(hp_ipc_state::irq_3));
	hpib.dio_read_cb().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	hpib.dio_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));
	hpib.eoi_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	hpib.dav_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	hpib.nrfd_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	hpib.ndac_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	hpib.ifc_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	hpib.srq_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));
	hpib.atn_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	hpib.ren_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));

	ieee488_device &ieee(IEEE488(config, IEEE488_TAG));
	ieee.eoi_callback().set("hpib" , FUNC(tms9914_device::eoi_w));
	ieee.dav_callback().set("hpib" , FUNC(tms9914_device::dav_w));
	ieee.nrfd_callback().set("hpib" , FUNC(tms9914_device::nrfd_w));
	ieee.ndac_callback().set("hpib" , FUNC(tms9914_device::ndac_w));
	ieee.ifc_callback().set("hpib" , FUNC(tms9914_device::ifc_w));
	ieee.srq_callback().set("hpib" , FUNC(tms9914_device::srq_w));
	ieee.atn_callback().set("hpib" , FUNC(tms9914_device::atn_w));
	ieee.ren_callback().set("hpib" , FUNC(tms9914_device::ren_w));
	IEEE488_SLOT(config , "ieee_rem" , 0 , remote488_devices , nullptr);

	// Beeper
	COP452(config , m_spkr , 2_MHz_XTAL);
	SPEAKER(config, "mono").front_center();
	DAC_1BIT(config, m_dac , 0).add_route(ALL_OUTPUTS, "mono", 0.5, 0);
	m_spkr->oa_w().set(m_dac , FUNC(dac_1bit_device::write));

	// IO slots
	HP_IPC_IO_SLOT(config , m_io_slot_a).set_slot_idx(0);
	HP_IPC_IO_SLOT(config , m_io_slot_b).set_slot_idx(1);

	// IRQ3/4/5/6 mergers
	INPUT_MERGER_ANY_HIGH(config , m_irq3_merger).output_handler().set_inputline(m_maincpu , M68K_IRQ_3);
	INPUT_MERGER_ANY_HIGH(config , m_irq4_merger).output_handler().set_inputline(m_maincpu , M68K_IRQ_4);
	INPUT_MERGER_ANY_HIGH(config , m_irq5_merger).output_handler().set_inputline(m_maincpu , M68K_IRQ_5);
	INPUT_MERGER_ANY_HIGH(config , m_irq6_merger).output_handler().set_inputline(m_maincpu , M68K_IRQ_6);
	m_io_slot_a->irq3_cb().set(m_irq3_merger , FUNC(input_merger_any_high_device::in_w<1>));
	m_io_slot_a->irq4_cb().set(m_irq4_merger , FUNC(input_merger_any_high_device::in_w<1>));
	m_io_slot_a->irq5_cb().set(m_irq5_merger , FUNC(input_merger_any_high_device::in_w<1>));
	m_io_slot_a->irq6_cb().set(m_irq6_merger , FUNC(input_merger_any_high_device::in_w<1>));
	m_io_slot_b->irq3_cb().set(m_irq3_merger , FUNC(input_merger_any_high_device::in_w<2>));
	m_io_slot_b->irq4_cb().set(m_irq4_merger , FUNC(input_merger_any_high_device::in_w<2>));
	m_io_slot_b->irq5_cb().set(m_irq5_merger , FUNC(input_merger_any_high_device::in_w<2>));
	m_io_slot_b->irq6_cb().set(m_irq6_merger , FUNC(input_merger_any_high_device::in_w<2>));

	// Optional ROMs
	for (auto& finder : m_rom_slots) {
		HP_IPC_OPTROM(config, finder);
	}

	RAM(config, RAM_TAG).set_default_size("512K").set_extra_options("768K,1M,1576K,2M,3M,4M,5M,6M,7M,7680K");
}

void hp_ipc_state::hp_ipc(machine_config &config)
{
	hp_ipc_base(config);

	ADDRESS_MAP_BANK(config, "bankdev").set_map(&hp_ipc_state::hp_ipc_mem_inner_9807a).set_options(ENDIANNESS_BIG, 16, 25, 0x1000000);

	// 16kw/32kb video RAM
	m_gpu->set_vram_size(16);

	// horizontal time = 60 us (min)
	// ver.refresh period = ~300 us
	// ver.period = 16.7ms (~60 hz)
	SCREEN(config , m_screen , SCREEN_TYPE_LCD); // actually an EL display
	m_screen->set_color(rgb_t::amber());
	m_screen->set_screen_update("gpu" , FUNC(hp1ll3_device::screen_update));
	m_screen->set_raw(6_MHz_XTAL * 2 , 720 , 0 , 512 , 261 , 0 , 255);
	m_screen->screen_vblank().set("mlc", FUNC(hp_hil_mlc_device::ap_w)); // this signal is actually driven by a 555 (U59)
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

void hp_ipc_state::hp9808a(machine_config &config)
{
	hp_ipc_base(config);

	ADDRESS_MAP_BANK(config, "bankdev").set_map(&hp_ipc_state::hp_ipc_mem_inner_9808a).set_options(ENDIANNESS_BIG, 16, 25, 0x1000000);

	// 64kw/128kb video RAM
	m_gpu->set_vram_size(64);

	SCREEN(config , m_screen , SCREEN_TYPE_LCD);
	m_screen->set_color(rgb_t::amber()); // actually a kind of EL display
	m_screen->set_screen_update("gpu" , FUNC(hp1ll3_device::screen_update));
	m_screen->set_raw(6_MHz_XTAL * 2 , 880 , 0 , 640 , 425 , 0 , 400);
	m_screen->screen_vblank().set("mlc", FUNC(hp_hil_mlc_device::ap_w));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
}


ROM_START(hp_ipc)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_DEFAULT_BIOS("v50")

	ROM_SYSTEM_BIOS(0, "v10", "HP-UX 1.0")
	ROMX_LOAD("00095-60006.bin", 0x00000, 0x80000, NO_DUMP, ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v50", "HP-UX 5.0")
	// Should be spread across 4 x 128K ROMs
	ROMX_LOAD("hp ipc os 82991a.bin", 0x00000, 0x80000, BAD_DUMP CRC(df45a37b) SHA1(476af9923bca0d2d0f40aeb81be5145ca76fddf5), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "diaga", "Diag ROM A")
	ROMX_LOAD("boarda_u1.bin", 0x00001, 0x4000, CRC(f40e1434) SHA1(a4f633f3e0971ba3ff218c6d6f777b8253bade8c), ROM_BIOS(2) | ROM_SKIP(1))
	ROMX_LOAD("boarda_u2.bin", 0x00000, 0x4000, CRC(80f6eb08) SHA1(ea943f72d37b43b5ba06b8e6f11824601109497c), ROM_BIOS(2) | ROM_SKIP(1))
	ROMX_LOAD("boarda_u3.bin", 0x40001, 0x4000, CRC(0f2fd4d5) SHA1(9ed41fca947d58d7f4159336421601963e3cae17), ROM_BIOS(2) | ROM_SKIP(1))
	ROMX_LOAD("boarda_u4.bin", 0x40000, 0x4000, CRC(d6741772) SHA1(1574a52d6658f9a7beace6572bea11ee923fc1bf), ROM_BIOS(2) | ROM_SKIP(1))

	ROM_SYSTEM_BIOS(3, "diagb", "Diag ROM B")
	ROMX_LOAD("boardb_u1.bin", 0x00001, 0x4000, CRC(597777d9) SHA1(24671499e4685f306b1b37a071adc0634f9f1e4e), ROM_BIOS(3) | ROM_SKIP(1))
	ROMX_LOAD("boardb_u2.bin", 0x00000, 0x4000, CRC(bc2ada19) SHA1(efebab335ca8a57e77b1076050e861a3a0eb09fc), ROM_BIOS(3) | ROM_SKIP(1))
	ROMX_LOAD("boardb_u3.bin", 0x40001, 0x2000, CRC(c227622a) SHA1(99747945dc6efaa45ce8d7d3c58830dbe46b651a), ROM_BIOS(3) | ROM_SKIP(1))
	ROMX_LOAD("boardb_u4.bin", 0x40000, 0x2000, CRC(d99efe90) SHA1(345437007f8d728ceafc5cd97414ef94ce38e363), ROM_BIOS(3) | ROM_SKIP(1))
ROM_END

#define rom_hp9808a rom_hp_ipc

} // Anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY            FULLNAME                            FLAGS
COMP( 1985, hp_ipc,  0,      0,      hp_ipc,  hp_ipc, hp_ipc_state, empty_init, "Hewlett-Packard", "Integral Personal Computer 9807A", 0)
COMP( 1985, hp9808a, 0,      0,      hp9808a, hp_ipc, hp_ipc_state, empty_init, "Hewlett-Packard", "Integral Personal Computer 9808A", MACHINE_NOT_WORKING)
