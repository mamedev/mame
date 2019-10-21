// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, R. Belmont, Ryan Holtz
/***************************************************************************

        Sun-4 Models
        ------------

    4/260
        Processor(s):   SF9010 @ 16.67MHz, Weitek 1164/1165, Sun-4 MMU,
                        16 hardware contexts
        Speed ratings:  10 MIPS, 1.6 MFLOPS
        CPU:            501-1274/1491/1522
        Chassis type:   deskside
        Bus:            VME (12 slot)
        Memory:         128M (documented) physical with ECC, 1G/process virtual,
                        60ns cycle
        Architecture:   sun4
        Notes:          First SPARC machine. Code-named "Sunrise". Cache
                        much like Sun-3/2xx, uses same memory boards.
                        May be upgraded 3/260.

    4/110
        Processor(s):   MB86900 @ 14.28MHz, Weitek 1164/1165, Sun-4 MMU,
                        16 hardware contexts
        Speed ratings:  7 MIPS
        CPU:            501-1199/1237/1462/1463/1464/1465/1512/1513/
                            1514/1515/1516/1517/1656/1657/1658/1659/
                            1660/1661
        Chassis type:   deskside
        Bus:            VME (3 slot), P4
        Memory:         32M physical with parity, 1G/process virtual,
                        70ns cycle
        Architecture:   sun4
        Notes:          First desktop-able SPARC. CPU doesn't support
                        VME busmaster cards (insufficient room on CPU
                        board for full VME bus interface), so DMA disk
                        and tape boards won't work with it. Originally
                        intended as single-board machine, although there
                        are a few slave-only VME boards (such as the
                        ALM-2 and second ethernet controller) which work
                        with it. SIMM memory (static column?).
                        Code-named "Cobra". CPUs 501-1199/1462/1464/1512/
                        1514/1516/1656/1658/1660 do not have an FPU;
                        501-1237/1463/1465/1513/1515/1517/1657/1659/1661
                        have an FPU.

    4/280
        Chassis type:   rackmount
        Notes:          Rackmount version of 4/260. May be upgraded
                        3/280.

    4/150
        Chassis type:   deskside
        Bus:            VME (6 slot)
        Notes:          See 4/110.

    SPARCstation 1 (4/60)
        Processor(s):   MB86901A or LSI L64801 @ 20MHz, Weitek 3170,
                        Sun-4c MMU, 8 hardware contexts
        Speed ratings:  12.5 MIPS, 1.4 MFLOPS, 10 SPECmark89
        CPU:            501-1382/1629
        Chassis type:   square pizza box
        Bus:            SBus @ 20MHz (3 slots, slot 3 slave-only)
        Memory:         64M physical with synchronous parity,
                        512M/process virtual, 50 ns cycle
        Cache:          64K write-through, direct-mapped, virtually
                        indexed, virtually tagged, 16-byte lines
        Architecture:   sun4c
        Notes:          Code name "Campus". SIMM memory. 3.5" floppy.
                        First supported in SunOS 4.0.3c.

    SPARCserver 1
        Notes:          SPARCstation 1 without a monitor/framebuffer.

    4/330 (SPARCstation 330, SPARCserver 330)
        Processor(s):   CY7C601 @ 25MHz, TI8847, Sun-4 MMU, 16 hardware
                        contexts
        Speed ratings:  16 MIPS, 2.6 MFLOPS, 11.3 SPECmark89
        CPU:            501-1316/1742
        Bus:            VME (3 9U slots, 2 each 6U and 3U), P4
        Memory:         56M/72M (documented) physical with synchronous
                        parity, 1G/process virtual, 40ns cycle
        Cache:          128K
        Architecture:   sun4
        Notes:          SIMM memory. Cache similar to 4/2xx but
                        write-through. Code-named "Stingray". 56M limit
                        only for early versions of ROM.

    4/310
        Chassis:        deskside
        Bus:            VME (3 slots), P4
        Notes:          See 4/330.

    4/350
        Chassis:        deskside
        Bus:            VME (6 slots), P4
        Notes:          See 4/330.

    4/360
        Notes:          4/260 upgraded with a 4/3xx CPU and memory boards.

    4/370 (SPARCstation 370, SPARCserver 370)
        Chassis:        deskside
        Bus:            VME (12 slots), P4
        Notes:          See 4/330.

    4/380
        Notes:          4/280 upgraded with a 4/3xx CPU and memory boards..

    4/390 (SPARCserver 390)
        Chassis:        rackmount
        Bus:            VME (16 slots)
        Notes:          See 4/330.

    4/470 (SPARCstation 470, SPARCserver 470)
        Processor(s):   CY7C601 @ 33MHz, TI8847 (?), 64 MMU hardware
                        contexts
        Speed ratings:  22 MIPS, 3.8 MFLOPS, 17.6 SPECmark89
        CPU:            501-1381/1899
        Chassis:        deskside
        Bus:            VME (12 slots), P4
        Memory:         96M (documented) physical
        Cache:          128K
        Architecture:   sun4
        Notes:          Write-back rather than write-through cache,
                        3-level rather than 2-level Sun-style MMU.
                        Code-name "Sunray" (which was also the code name
                        for the 7C601 CPU).

    4/490 (SPARCserver 490)
        Chassis:        rackmount
        Bus:            VME (16 slots), P4
        Notes:          See 4/470.

    SPARCstation SLC (4/20)
        Processor(s):   MB86901A or LSI L64801 @ 20MHz
        Speed ratings:  12.5 MIPS, 1.2 MFLOPS, 8.6 SPECmark89
        CPU:            501-1627/1680/1720/1748 (1776/1777 ?)
        Chassis type:   monitor
        Bus:            none
        Memory:         16M physical
        Cache:          64K write-through, direct-mapped, virtually
                        indexed, virtually tagged, 16-byte lines
        Architecture:   sun4c
        Notes:          Code name "Off-Campus". SIMM memory. No fan.
                        Built into 17" mono monitor. First supported in
                        SunOS 4.0.3c.

    SPARCstation IPC (4/40)
        Processor(s):   MB86901A or LSI L64801 @ 25MHz
        Speed ratings:  13.8 SPECint92, 11.1 SPECfp92, 327
                        SPECintRate92, 263 SPECfpRate92
        CPU:            501-1689/1835/1870/1974 (1690?)
        Chassis type:   lunchbox
        Bus:            SBus @ 25MHz (2 slots)
        Memory:         48M physical
        Cache:          64K write-through, direct-mapped, virtually
                        indexed, virtually tagged, 16-byte lines
        Architecture:   sun4c
        Notes:          Code name "Phoenix". SIMM memory. Onboard mono
                        framebuffer. 3.5" floppy. First supported in
                        SunOS 4.0.3c.

    SPARCstation 1+ (4/65)
        Processor(s):   LSI L64801 @ 25MHz, Weitek 3172, Sun-4c MMU,
                        8 hardware contexts
        Speed ratings:  15.8 MIPS, 1.7 MFLOPS, 12 SPECmark89
        CPU:            501-1632
        Chassis type:   square pizza box
        Bus:            SBus @ 25MHz (3 slots, slot 3 slave-only)
        Memory:         64M (40M?) physical with synchronous parity,
                        512M/process virtual, 50ns cycle
        Cache:          64K write-through, direct-mapped, virtually
                        indexed, virtually tagged, 16-byte lines
        Architecture:   sun4c
        Notes:          Code name "Campus B". SIMM memory. 3.5" floppy.
                        Essentially same as SPARCstation 1, just faster
                        clock and improved SCSI controller. First
                        supported in SunOS 4.0.3c.

    SPARCserver 1+
        Notes:          SPARCstation 1+ without a monitor/framebuffer.

    SPARCstation 2 (4/75)
        Processor(s):   CY7C601 @ 40MHz, TI TMS390C601A (602A ?), Sun-4c
                        MMU, 16 hardware contexts
        Speed ratings:  28.5 MIPS, 4.2 MFLOPS, 21.8 SPECint92, 22.8
                        SPECfp92, 517 SPECintRate92, 541 SPECfpRate92
        CPU:            501-1638/1744/1858/1859/1912/1926/1989/1995
        Chassis type:   square pizza box
        Bus:            SBus @ 20MHz (3 slots)
        Memory:         64M physical on motherboard/128M total
        Cache:          64K write-through, direct-mapped, virtually
                        indexed, virtually tagged, 32-byte lines
        Architecture:   sun4c
        Notes:          Code name "Calvin". SIMMs memory. 3.5" floppy.
                        Case slightly larger and has more ventilation.
                        (Some models apparently have LSI L64811 @
                        40MHz?) Expansion beyond 64M is possible with a
                        32M card which can take a 32M daughterboard
                        (card blocks SBus slot). First supported in
                        SunOS 4.1.1.

    SPARCserver 2
        Notes:          SPARCstation 2 without a monitor/framebuffer.

    SPARCstation ELC (4/25)
        Processor(s):   Fujitsu MB86903 or Weitek W8701 @ 33MHz, FPU on
                        CPU chip, Sun-4c MMU, 8 hardware contexts
        Speed ratings:  21 MIPS, 3 MFLOPS, 18.2 SPECint92, 17.9
                        SPECfp92, 432 SPECintRate92, 425 SPECfpRate92
        CPU:            501-1861 (1730?)
        Chassis type:   monitor
        Bus:            none
        Memory:         64M physical
        Cache:          64K write-through, direct-mapped, virtually
                        indexed, virtually tagged, 32-byte lines
        Architecture:   sun4c
        Notes:          Code name "Node Warrior". SIMM memory. No fan.
                        Built into 17" mono monitor. first supported in
                        SunOS 4.1.1c.

    SPARCstation IPX (4/50)
        Processor(s):   Fujitsu MB86903 or Weitek W8701 @ 40MHz, FPU on
                        CPU chip, Sun-4c MMU, 8 hardware contexts
        Speed ratings:  28.5 MIPS, 4.2 MFLOPS, 21.8 SPECint92,
                        21.5 SPECfp92, 517 SPECintRate92, 510
                        SPECfpRate92
        CPU:            501-1780/1810/1959/2044
        Chassis type:   lunchbox
        Bus:            SBus @ 20MHz (2 slots)
        Memory:         64M physical
        Cache:          64K write-through cache, direct-mapped,
                        virtually indexed, virtually tagged, 32-byte
                        lines
        Architecture:   sun4c
        Notes:          Code name "Hobbes". SIMM memory. Onboard
                        GX-accelerated cg6 color framebuffer (not usable
                        with ECL mono monitors, unlike SBus version).
                        Picture of Hobbes (from Watterson's "Calvin and
                        Hobbes" comic strip) silkscreened on
                        motherboard. 3.5" floppy. First supported in
                        SunOS 4.1.1 (may require IPX supplement).

    SPARCengine 1 (4/E)
        CPU:            501-8035/8058/8064
        Bus:            VME (6U form factor), SBus (1 slot)
        Notes:          Single-board VME SPARCstation 1 (or 1+?),
                        presumably for use as a controller, not as a
                        workstation. 8K MMU pages rather than 4K.
                        External RAM, framebuffer, and SCSI/ethernet
                        boards available. Code name "Polaris".

    SPARCserver 630MP (4/630)
        Processor(s):   MBus modules
        CPU:            501-1686/2055
        Chassis type:   deskside
        Bus:            VME (3 9U slots, 2 each 6U and 3U), SBus @ 20MHz
                        (4 slots), MBus (2 slots)
        Memory:         640M physical
        Architecture:   sun4m
        Notes:          First MBus-based machine. Code name "Galaxy".
                        SIMM memory.

    SPARCserver 670MP (4/670)
        Chassis type:   deskside
        Bus:            VME (12 slots), SBus @ 20MHz (4 slots), MBus (2
                        slots)
        Notes:          Like SPARCserver 630MP. More SBus slots can be
                        added via VME expansion boards.

    SPARCserver 690MP (4/690)
        Chassis type:   rackmount
        Bus:            VME (16 slots), SBus @ 20MHz (4 slots), MBus (2
                        slots)
        Notes:          See SPARCserver 670MP.

    SPARCclassic (SPARCclassic Server)(SPARCstation LC) (4/15)
        Processor(s):   microSPARC @ 50MHz
        Speed ratings:  59.1 MIPS, 4.6 MFLOPS, 26.4 SPECint92, 21.0
                        SPECfp92, 626 SPECintRate92, 498 SPECfpRate92
        CPU:            501-2200/2262/2326
        Chassis type:   lunchbox
        Bus:            SBus @ 20MHz (2 slots)
        Memory:         96M physical
        Architecture:   sun4m
        Notes:          Sun4m architecture, but no MBus (uniprocessor
                        only). SIMM memory. Shares code name "Sunergy"
                        with LX. 3.5" floppy. Soldered CPU chip. Onboard
                        cgthree framebuffer, AMD79C30 8-bit audio chip.
                        First supported in SunOS 4.1.3c.

    SPARCclassic X (4/10)
        CPU:            501-2079/2262/2313
        Notes:          Essentially the same as SPARCclassic, but
                        intended for use as an X terminal (?).

    SPARCstation LX/ZX (4/30)
        Processor(s):   microSPARC @ 50MHz
        Speed ratings:  59.1 MIPS, 4.6 MFLOPS, 26.4 SPECint92, 21.0
                        SPECfp92, 626 SPECintRate92, 498 SPECfpRate92
        CPU:            501-2031/2032/2233/2474
        Chassis type:   lunchbox
        Bus:            SBus @ 20MHz (2 slots)
        Memory:         96M physical
        Architecture:   sun4m
        Notes:          Sun4m architecture, but no MBus (uniprocessor
                        only). SIMM memory. Shares code name "Sunergy"
                        with SPARCclassic. Soldered CPU chip. Onboard
                        cgsix framebuffer, 1M VRAM standard, expandable
                        to 2M. DBRI 16-bit audio/ISDN chip. First
                        supported in SunOS 4.1.3c.

   SPARCstation Voyager
        Processors(s):  microSPARC II @ 60MHz
        Speed ratings:  47.5 SPECint92, 40.3 SPECfp92, 1025
                        SPECintRate92, 859 SPECfpRate92
        Bus:            SBus; PCMCIA type II (2 slots)
        Memory:         80M physical
        Architecture:   sun4m
        Notes:          Portable (laptop?). 16M standard, two memory
                        expansion slots for Voyager-specific SIMMs (16M
                        or 32M). Code-named "Gypsy". 14" 1152x900 mono
                        or 12" 1024x768 color flat panel displays. DBRI
                        16-bit audio/ISDN chip.

    SPARCstation 3
        Notes:          Although this model appeared in a few Sun price
                        lists, it was renamed the SPARCstation 10 before
                        release.

    SPARCstation 10/xx
        Processor(s):   MBus modules
        Motherboard:    501-1733/2259/2274/2365 (-2274 in model 20 only)
        Chassis type:   square pizza box
        Bus:            SBus @ 16.6/20MHz (model 20) or 18/20MHz (other
                        models) (4 slots); MBus (2 slots)
        Memory:         512M physical
        Architecture:   sun4m
        Notes:          Code name "Campus-2". 3.5" floppy. SIMM memory.
                        Some models use double-width MBus modules which
                        block SBus slots. Also, the inner surface of the
                        chassis is conductive, so internal disk drives
                        must be mounted with insulating hardware.

    SPARCserver 10/xx
        Notes:          SPARCstation 10/xx without monitor/framebuffer.

    SPARCcenter 2000
        Processor(s):   MBus modules
        Motherboard:    501-1866/2334/2362
        Bus:            XDBus * 2 (20 slots); SBus @ 20MHz (4
                        slots/motherboard); MBus (2 slots/motherboard)
        Memory:         5G physical
        Cache:          2M/motherboard
        Architecture:   sun4d
        Notes:          Dual XDBus backplane with 20 slots. One board
                        type that carries dual MBus modules with 2M
                        cache (1M for each XDBus), 512M memory and 4
                        SBus slots. Any combination can be used; memory
                        is *not* tied to the CPU modules but to an
                        XDBus. Solaris 2.x releases support an
                        increasing number of CPUs (up to twenty), due to
                        tuning efforts in the kernel. First supported in
                        Solaris 2.2 (SunOS 5.2). Code name "Dragon".

    SPARCserver 1000
        Processor(s):   MBus modules
        Motherboard:    501-2336 (2338?)
        Bus:            XDBus; SBus @ 20MHz (3 slots/motherboard); MBus
                        (2 slots/motherboard)
        Memory:         2G physical
        Cache:          1M/motherboard
        Architecture:   sun4d
        Notes:          Single XDBus design with "curious L-shaped
                        motherboards". Three SBus slots per motherboard,
                        512M, two MBus modules per motherboard. Four
                        motherboards total, or a disk tray with four 1"
                        high 3.5" disks. Code name "Scorpion". First
                        supported in Solaris 2.2 (SunOS 5.2).

        21/11/2011 Skeleton driver.
        20/06/2016 Much less skeletony.

        // sun4:  16 contexts, 4096 segments, each PMEG is 32 PTEs, each PTE is 8K
        // VA lower 13 bits in page, next 5 bits select PTE in PMEG, next 12 bits select PMEG, top 2 must be 00 or 11.

        4/60 ROM notes:

        ffe809fc: call to print "Sizing Memory" to the UART
        ffe80a70: call to "Setting up RAM for monitor" that goes wrong
        ffe80210: testing memory
        ffe80274: loop that goes wobbly and fails
        ffe80dc4: switch off boot mode, MMU maps ROM to copy in RAM from here on
        ffe82000: start of FORTH (?) interpreter once decompressed
                  text in decompressed area claims to be FORTH-83 FCode, but the opcodes
                  do not match the documented OpenFirmware FCode ones at all.

        4/3xx ROM notes:
        sun4: CPU LEDs to 00 (PC=ffe92398) => ........
        sun4: CPU LEDs to 01 (PC=ffe92450) => *.......
        sun4: CPU LEDs to 02 (PC=ffe9246c) => .*......
        sun4: CPU LEDs to 03 (PC=ffe9aa54) => **......
        sun4: CPU LEDs to 04 (PC=ffe9aa54) => ..*.....
        sun4: CPU LEDs to 05 (PC=ffe9aa54) => *.*.....
        sun4: CPU LEDs to 06 (PC=ffe9aa54) => .**.....
        sun4: CPU LEDs to 07 (PC=ffe9aa54) => ***.....


****************************************************************************/

#include "emu.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"
#include "bus/sunkbd/sunkbd.h"
#include "bus/sunmouse/sunmouse.h"
#include "bus/sbus/sbus.h"
#include "bus/sbus/bwtwo.h"
#include "cpu/sparc/sparc.h"
#include "imagedev/floppy.h"
#include "machine/am79c90.h"
#include "machine/bankdev.h"
#include "machine/ncr5390.h"
#include "machine/nscsi_bus.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/sun4c_mmu.h"
#include "machine/timekpr.h"
#include "machine/upd765.h"
#include "machine/z80scc.h"

#include "debug/debugcon.h"
#include "debug/debugcmd.h"
#include "debugger.h"
#include "screen.h"

#include "formats/mfi_dsk.h"
#include "formats/pc_dsk.h"

#define SUN4_LOG_FCODES (0)

#define TIMEKEEPER_TAG  "timekpr"
#define SCC1_TAG        "scc1"
#define SCC2_TAG        "scc2"
#define KEYBOARD_TAG    "keyboard"
#define MOUSE_TAG       "mouseport"
#define RS232A_TAG      "rs232a"
#define RS232B_TAG      "rs232b"
#define FDC_TAG         "fdc"
#define LANCE_TAG       "lance"

// DMA controller constants
#define DMA_DEV_ID      (0x80000000)
#define DMA_L           (0x00008000)    // use ILACC
#define DMA_TC          (0x00004000)    // terminal count
#define DMA_EN_CNT      (0x00002000)    // enable count
#define DMA_BYTE_ADDR   (0x00001800)    // next byte number to be accessed
#define DMA_BYTE_ADDR_SHIFT (11)
#define DMA_REQ_PEND    (0x00000400)    // request pending
#define DMA_EN_DMA      (0x00000200)    // enable DMA
#define DMA_WRITE       (0x00000100)    // DMA device->mem if 1, otherwise mem->device
#define DMA_RESET       (0x00000080)    // DMA hardware reset
#define DMA_DRAIN       (0x00000040)    // force remaining pack bytes to memory
#define DMA_FLUSH       (0x00000020)    // force PACK_CNT and ERR_PEND to 0
#define DMA_INT_EN      (0x00000010)    // interrupt enable
#define DMA_PACK_CNT    (0x0000000c)    // number of bytes in pack register
#define DMA_PACK_CNT_SHIFT  (2)
#define DMA_ERR_PEND    (0x00000002)    // error pending, set when memory exception occurs
#define DMA_INT_PEND    (0x00000001)    // interrupt pending, set when TC=1
#define DMA_READ_ONLY   (DMA_TC | DMA_BYTE_ADDR | DMA_REQ_PEND | DMA_PACK_CNT | DMA_ERR_PEND | DMA_INT_PEND)
#define DMA_WRITE_ONLY  (DMA_FLUSH)
#define DMA_READ_WRITE  (DMA_EN_CNT | DMA_EN_DMA | DMA_WRITE | DMA_RESET | DMA_DRAIN | DMA_INT_EN)
#define DMA_CTRL        (0)
#define DMA_ADDR        (1)
#define DMA_BYTE_COUNT  (2)
#define DMA_XTAL        (25_MHz_XTAL)

#define AUXIO_DENSITY   (0x20)
#define AUXIO_DISK_CHG  (0x10)
#define AUXIO_DRIVE_SEL (0x08)
#define AUXIO_TC        (0x04)
#define AUXIO_EJECT     (0x02)
#define AUXIO_LED       (0x01)

namespace
{
const sparc_disassembler::asi_desc_map::value_type sun4_asi_desc[] = {
														{ 0x10, { nullptr, "Flush I-Cache (Segment)" } },
														{ 0x11, { nullptr, "Flush I-Cache (Page)"    } },
	{ 0x02, { nullptr, "System Space"           } }, { 0x12, { nullptr, "Flush I-Cache (Context)" } },
	{ 0x03, { nullptr, "Segment Map"            } }, { 0x13, { nullptr, "Flush I-Cache (User)"    } },
	{ 0x04, { nullptr, "Page Map"               } }, { 0x14, { nullptr, "Flush D-Cache (Segment)" } },
	{ 0x05, { nullptr, "Block Copy"             } }, { 0x15, { nullptr, "Flush D-Cache (Page)"    } },
	{ 0x06, { nullptr, "Region Map"             } }, { 0x16, { nullptr, "Flush D-Cache (Context)" } },
	{ 0x07, { nullptr, "Flush Cache (Region)"   } }, { 0x17, { nullptr, "Flush D-Cache (User)"    } },
	{ 0x08, { nullptr, "User Instruction"       } },
	{ 0x09, { nullptr, "Supervisor Instruction" } },
	{ 0x0a, { nullptr, "User Data"              } },
	{ 0x0b, { nullptr, "Supervisor Data"        } }, { 0x1b, { nullptr, "Flush I-Cache (Region)"  } },
	{ 0x0c, { nullptr, "Flush Cache (Segment)"  } },
	{ 0x0d, { nullptr, "Flush Cache (Page)"     } },
	{ 0x0e, { nullptr, "Flush Cache (Context)"  } },
	{ 0x0f, { nullptr, "Flush Cache (User)"     } }, { 0x1f, { nullptr, "Flush D-Cache (Region)"  } }
};

const sparc_disassembler::asi_desc_map::value_type sun4c_asi_desc[] = {
	{ 0x02, { nullptr, "System Space"           } },
	{ 0x03, { nullptr, "Segment Map"            } },
	{ 0x04, { nullptr, "Page Map"               } },
	{ 0x08, { nullptr, "User Instruction"       } },
	{ 0x09, { nullptr, "Supervisor Instruction" } },
	{ 0x0a, { nullptr, "User Data"              } },
	{ 0x0b, { nullptr, "Supervisor Data"        } },
	{ 0x0c, { nullptr, "Flush Cache (Segment)"  } },
	{ 0x0d, { nullptr, "Flush Cache (Page)"     } },
	{ 0x0e, { nullptr, "Flush Cache (Context)"  } }
};
}

enum
{
	ARCH_SUN4 = 0,
	ARCH_SUN4C,
	ARCH_SUN4E
};

class sun4_state : public driver_device
{
public:
	sun4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mmu(*this, "mmu")
		, m_timekpr(*this, TIMEKEEPER_TAG)
		, m_scc1(*this, SCC1_TAG)
		, m_scc2(*this, SCC2_TAG)
		, m_fdc(*this, FDC_TAG)
		, m_floppy(*this, FDC_TAG":0")
		, m_lance(*this, LANCE_TAG)
		, m_scsibus(*this, "scsibus")
		, m_scsi(*this, "scsibus:7:ncr53c90a")
		, m_sbus(*this, "sbus")
		, m_sbus_slot(*this, "slot%u", 1U)
		, m_type0space(*this, "type0")
		, m_type1space(*this, "type1")
		, m_ram(*this, RAM_TAG)
		, m_rom(*this, "user1")
		, m_rom_ptr(nullptr)
	{
	}

	void sun4c(machine_config &config);
	void sun4_20(machine_config &config);
	void sun4_40(machine_config &config);
	void sun4_50(machine_config &config);
	void sun4_60(machine_config &config);
	void sun4_65(machine_config &config);
	void sun4_75(machine_config &config);
	void sun4(machine_config &config);

	void init_sun4();
	void init_sun4c();

private:
	virtual void machine_reset() override;
	virtual void machine_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static const device_timer_id TIMER_0 = 0;
	static const device_timer_id TIMER_1 = 1;
	static const device_timer_id TIMER_RESET = 2;

	enum insn_data_mode
	{
		USER_INSN,
		SUPER_INSN,
		USER_DATA,
		SUPER_DATA
	};

	enum
	{
		// system enable constants
		ENA_NOTBOOT     = 0x80,
		ENA_SDVMA       = 0x20,
		ENA_CACHE       = 0x10,
		ENA_RESET       = 0x04,
		ENA_DIAG        = 0x01,

		// page table entry constants
		PM_VALID        = 0x80000000,    // page is valid
		PM_WRITEMASK    = 0x40000000,    // writable?
		PM_SYSMASK      = 0x20000000,    // system use only?
		PM_CACHE        = 0x10000000,    // cachable?
		PM_TYPEMASK     = 0x0c000000,    // type mask
		PM_ACCESSED     = 0x02000000,    // accessed flag
		PM_MODIFIED     = 0x01000000     // modified flag
	};

	DECLARE_READ32_MEMBER( sun4_cache_flush_r );
	DECLARE_WRITE32_MEMBER( sun4_cache_flush_w );
	DECLARE_READ32_MEMBER( sun4_system_r );
	DECLARE_WRITE32_MEMBER( sun4_system_w );
	DECLARE_READ32_MEMBER( sun4_segment_map_r );
	DECLARE_WRITE32_MEMBER( sun4_segment_map_w );
	DECLARE_READ32_MEMBER( sun4_page_map_r );
	DECLARE_WRITE32_MEMBER( sun4_page_map_w );
	template <insn_data_mode MODE> DECLARE_READ32_MEMBER( sun4_insn_data_r );
	DECLARE_WRITE32_MEMBER( sun4_insn_data_w );

	DECLARE_READ32_MEMBER( sun4c_debugger_r );
	DECLARE_WRITE32_MEMBER( sun4c_debugger_w );

	DECLARE_READ32_MEMBER( sun4_mmu_r );
	DECLARE_WRITE32_MEMBER( sun4_mmu_w );
	DECLARE_READ32_MEMBER( sun4c_mmu_r );
	DECLARE_WRITE32_MEMBER( sun4c_mmu_w );
	DECLARE_READ32_MEMBER( ram_r );
	DECLARE_WRITE32_MEMBER( ram_w );
	DECLARE_READ32_MEMBER( timeout_r );
	DECLARE_WRITE32_MEMBER( timeout_w );
	DECLARE_READ32_MEMBER( ss1_sl0_id );
	DECLARE_READ32_MEMBER( timer_r );
	DECLARE_WRITE32_MEMBER( timer_w );
	DECLARE_READ8_MEMBER( irq_r );
	DECLARE_WRITE8_MEMBER( irq_w );
	DECLARE_READ8_MEMBER( fdc_r );
	DECLARE_WRITE8_MEMBER( fdc_w );
	DECLARE_READ8_MEMBER( auxio_r );
	DECLARE_WRITE8_MEMBER( auxio_w );
	DECLARE_READ32_MEMBER( dma_r );
	DECLARE_WRITE32_MEMBER( dma_w );
	DECLARE_WRITE32_MEMBER( buserr_w );

	DECLARE_WRITE_LINE_MEMBER( scsi_irq );
	DECLARE_WRITE_LINE_MEMBER( scsi_drq );

	DECLARE_WRITE_LINE_MEMBER( scc1_int );
	DECLARE_WRITE_LINE_MEMBER( scc2_int );

	DECLARE_WRITE_LINE_MEMBER( fdc_irq );

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	void ncr53c90a(device_t *device);

	void sun4_system_map(address_map &map);
	void sun4_segment_map(address_map &map);
	void sun4_page_map(address_map &map);
	void sun4_insn_map(address_map &map);
	void sun4_supervisor_insn_map(address_map &map);
	void sun4_data_map(address_map &map);
	void sun4_supervisor_data_map(address_map &map);

	void sun4c_debugger_map(address_map &map);

	void type0space_map(address_map &map);
	void type1space_map(address_map &map);
	void type1space_sbus_map(address_map &map);
	void type1space_s4_map(address_map &map);

	enum sun4_arch
	{
		SUN4 = 0,
		SUN4C = 1
	};

	required_device<mb86901_device> m_maincpu;
	required_device<sun4c_mmu_device> m_mmu;

	required_device<m48t02_device> m_timekpr;

	required_device<z80scc_device> m_scc1;
	required_device<z80scc_device> m_scc2;

	required_device<upd765_family_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_device<am79c90_device> m_lance;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr53c90a_device> m_scsi;

	optional_device<sbus_device> m_sbus;
	optional_device_array<sbus_slot_device, 3> m_sbus_slot;
	optional_device<address_map_bank_device> m_type0space;
	optional_device<address_map_bank_device> m_type1space;
	memory_access_cache<2, 0, ENDIANNESS_BIG> *m_type1_cache;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;

	struct page_entry_t
	{
		uint32_t valid;
		uint32_t writable;
		uint32_t supervisor;
		uint32_t uncached;
		uint32_t accessed;
		uint32_t modified;
		uint32_t page;
		uint8_t type;
		uint8_t pad[3];

		uint32_t to_uint()
		{
			return valid | writable | supervisor | uncached | (type << 26) | accessed | modified | (page >> 11);
		}

		void merge_uint(uint32_t data, uint32_t mem_mask)
		{
			const uint32_t new_value = (to_uint() & ~mem_mask) | (data & mem_mask);
			valid = new_value & PM_VALID;
			writable = new_value & PM_WRITEMASK;
			supervisor = new_value & PM_SYSMASK;
			uncached = new_value & PM_CACHE;
			type = (new_value & PM_TYPEMASK) >> 26;
			accessed = new_value & PM_ACCESSED;
			modified = new_value & PM_MODIFIED;
			page = (new_value & 0x7ffff) << 11;
		}
	};

	uint8_t m_segmap[16][4096];
	page_entry_t m_pagemap[16384];
	uint32_t m_cachetags[16384];
	uint32_t m_cachedata[16384];

	uint32_t *m_rom_ptr;
	uint32_t *m_ram_ptr;
	uint32_t m_ram_size;
	uint32_t m_ram_size_words;
	uint32_t m_context;
	uint32_t m_context_masked;
	uint8_t m_system_enable;
	bool m_fetch_bootrom;
	uint32_t m_buserr[4];

	uint32_t m_segmap_masked[16][4096];
	uint8_t *m_curr_segmap;
	uint32_t *m_curr_segmap_masked;
	bool m_page_valid[16384];

	uint8_t m_auxio;
	uint32_t m_counter[4];
	uint32_t m_dma[4];
	bool m_dma_irq;
	bool m_dma_tc_read;
	uint32_t m_dma_pack_register;
	int m_scsi_irq;
	int m_fdc_irq;

	uint8_t m_irq_reg;    // IRQ control
	uint8_t m_scc1_int, m_scc2_int;
	uint8_t m_diag;
	int m_arch;

	attotime m_c0_last_read_time;
	attotime m_c1_last_read_time;
	emu_timer *m_c0_timer;
	emu_timer *m_c1_timer;
	emu_timer *m_reset_timer;

	void dma_check_interrupts();
	void dma_transfer();
	void dma_transfer_write();
	void dma_transfer_read();

	void l2p_command(int ref, const std::vector<std::string> &params);
	void fcodes_command(int ref, const std::vector<std::string> &params);
};

READ32_MEMBER( sun4_state::sun4_system_r )
{
	switch (offset >> 26)
	{
		case 3: // context reg
			if (mem_mask == 0x00ff0000) return m_context<<16;
			return m_context<<24;

		case 4: // system enable reg
			return m_system_enable<<24;

		case 6: // bus error register
			//printf("sun4: read buserror, PC=%x (mask %08x)\n", m_maincpu->pc(), mem_mask);
			return 0;

		case 8: // (d-)cache tags
			//logerror("sun4: read dcache tags @ %x, PC = %x\n", offset, m_maincpu->pc());
			return m_cachetags[offset&0xfff];

		case 9: // (d-)cache data
			//logerror("sun4: read dcache data @ %x, PC = %x\n", offset, m_maincpu->pc());
			return 0xffffffff;

		case 0xf:   // UART bypass
			//printf("read UART bypass @ %x mask %08x (PC=%x)\n", offset<<2, mem_mask, m_maincpu->pc());
			switch (offset & 3)
			{
				case 0: if (mem_mask == 0xff000000) return m_scc2->cb_r(offset)<<24; else return m_scc2->db_r(offset)<<8; break;
				case 1: if (mem_mask == 0xff000000) return m_scc2->ca_r(offset)<<24; else return m_scc2->da_r(offset)<<8; break;
			}
			return 0xffffffff;

		case 0:
		default:
			//logerror("sun4: ASI 2 space unhandled read @ %x (PC=%x)\n", offset<<2, m_maincpu->pc());
			return 0;
	}
}

WRITE32_MEMBER( sun4_state::sun4_system_w )
{
	switch (offset >> 26)
	{
		case 3: // context reg
			//printf("%08x to context, mask %08x, offset %x\n", data, mem_mask, offset);
			m_context = data>>24;
			return;

		case 4: // system enable reg
			m_system_enable = data>>24;

			if (m_system_enable & ENA_RESET)
			{
				m_reset_timer->adjust(attotime::from_usec(1), 0);
				m_maincpu->set_input_line(SPARC_RESET, ASSERT_LINE);
				//logerror("%s: Asserting reset line\n", machine().describe_context());
			}
			//printf("%08x to system enable, mask %08x\n", data, mem_mask);
			if (m_system_enable & ENA_RESET)
			{
				m_system_enable = 0;
				m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
				m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			}
			return;

		case 7: // diag reg
			m_diag = data >> 24;
			#if 1
			printf("sun4: CPU LEDs to %02x (PC=%x) => ", ((data>>24) & 0xff) ^ 0xff, m_maincpu->pc());
			for (int i = 0; i < 8; i++)
			{
				if (m_diag & (1<<i))
				{
					printf(".");
				}
				else
				{
					printf("*");
				}
			}
			printf("\n");
			#endif
			return;

		case 8: // cache tags
			//logerror("sun4: %08x to cache tags @ %x, PC = %x\n", data, offset, m_maincpu->pc());
			m_cachetags[offset&0xfff] = data;
			return;

		case 9: // cache data
			//logerror("sun4: %08x to cache data @ %x, PC = %x\n", data, offset, m_maincpu->pc());
			return;

		case 0xf:   // UART bypass
			//printf("%08x to UART bypass @ %x, mask %08x\n", data, offset<<2, mem_mask);
			switch (offset & 3)
			{
				case 0: if (mem_mask == 0xff000000) m_scc2->cb_w(offset, data>>24); else m_scc2->db_w(offset, data>>8); break;
				case 1: if (mem_mask == 0xff000000) m_scc2->ca_w(offset, data>>24); else { m_scc2->da_w(offset, data>>8); printf("%c", data>>8); } break;
			}
			return;

		case 0: // IDPROM
		default:
			//logerror("sun4: ASI 2 space unhandled write %x @ %x (mask %08x, PC=%x)\n", data, offset<<2, mem_mask, m_maincpu->pc());
			return;
	}
}

READ32_MEMBER( sun4_state::sun4_segment_map_r )
{
	//printf("sun4: read segment map @ %x (ctx %d entry %d, mem_mask %08x, PC=%x)\n", offset << 2, m_context, (offset>>16) & 0xfff, mem_mask, m_maincpu->pc());
	if (mem_mask == 0xffff0000)
		return m_curr_segmap[(offset>>16) & 0xfff]<<16;
	else if (mem_mask == 0xff000000)
		return m_curr_segmap[(offset>>16) & 0xfff]<<24;
	else if (mem_mask == 0xffffffff)
		return m_curr_segmap[(offset>>16) & 0xfff];
	else
		logerror("sun4: read segment map w/ unknown mask %08x\n", mem_mask);
	return 0x0;
}

WRITE32_MEMBER( sun4_state::sun4_segment_map_w )
{
	uint8_t segdata = 0;
	//printf("segment write, mask %08x, PC=%x\n", mem_mask, m_maincpu->pc());
	if (mem_mask == 0xffff0000) segdata = (data >> 16) & 0xff;
	else if (mem_mask == 0xff000000) segdata = (data >> 24) & 0xff;
	else logerror("sun4: writing segment map with unknown mask %08x, PC=%x\n", mem_mask, m_maincpu->pc());

	//printf("sun4: %08x to segment map @ %x (ctx %d entry %d, mem_mask %08x, PC=%x)\n", segdata, offset << 2, m_context, (offset>>16) & 0xfff, mem_mask, m_maincpu->pc());
	m_curr_segmap[(offset>>16) & 0xfff] = segdata;
	m_curr_segmap_masked[(offset>>16) & 0xfff] = segdata << 5;
}

READ32_MEMBER( sun4_state::sun4_page_map_r )
{
	uint32_t page = m_curr_segmap_masked[(offset >> 16) & 0xfff] | ((offset >> 11) & 0x1f);
	//printf("sun4: read page map @ %x (entry %d, seg %d, PMEG %d, mem_mask %08x, PC=%x)\n", offset << 2, page, (offset >> 16) & 0xfff, m_segmap[m_context][(offset >> 16) & 0xfff] & m_pmeg_mask, mem_mask, m_maincpu->pc());
	return m_pagemap[page].to_uint();
}

WRITE32_MEMBER( sun4_state::sun4_page_map_w )
{
	const uint32_t page = m_curr_segmap_masked[(offset >> 16) & 0xfff] | ((offset >> 11) & 0x1f);
	//printf("sun4: %08x to page map @ %x (entry %d, mem_mask %08x, PC=%x)\n", data, offset << 2, page, mem_mask, m_maincpu->pc());
	m_pagemap[page].merge_uint(data, mem_mask);
	m_page_valid[page] = m_pagemap[page].valid;
}

READ32_MEMBER( sun4_state::sun4c_debugger_r )
{
	return m_mmu->insn_data_r<sun4c_mmu_device::SUPER_INSN>(offset, mem_mask);
}

WRITE32_MEMBER( sun4_state::sun4c_debugger_w )
{
	m_mmu->insn_data_w<sun4c_mmu_device::SUPER_INSN>(offset, data, mem_mask);
}

template <sun4_state::insn_data_mode MODE>
READ32_MEMBER( sun4_state::sun4_insn_data_r )
{
	// supervisor program fetches in boot state are special
	if (m_fetch_bootrom && MODE == SUPER_INSN)
	{
		return m_rom_ptr[offset & 0x1ffff];
	}

	// it's translation time
	const uint32_t pmeg = m_curr_segmap_masked[(offset >> 16) & 0xfff];
	const uint32_t entry_index = pmeg | ((offset >> 11) & 0x1f);

	if (m_page_valid[entry_index])
	{
		page_entry_t &entry = m_pagemap[entry_index];
		entry.accessed = PM_ACCESSED;

		const uint32_t tmp = entry.page | (offset & 0x7ff);

		//printf("sun4: read translated vaddr %08x to phys %08x type %d, PTE %08x, PC=%x\n", offset<<2, tmp<<2, entry.type, entry.to_uint(), m_maincpu->pc());

		switch (entry.type)
		{
		case 0: // type 0 space
			return m_type0space->read32(tmp, mem_mask);

		case 1: // type 1 space
			// magic EPROM bypass
			if ((tmp >= (0x6000000>>2)) && (tmp <= (0x6ffffff>>2)))
			{
				return m_rom_ptr[offset & 0x1ffff];
			}
			//printf("Read type 1 @ VA %08x, phys %08x\n", offset<<2, tmp<<2);
			return m_type1space->read32(tmp, mem_mask);

		default:
			//logerror("sun4: access to unhandled memory type\n");
			return 0;
		}
	}
	else
	{
		if (!machine().side_effects_disabled())
		{
			//logerror("sun4: INVALID PTE entry %d %08x accessed!  vaddr=%x PC=%x\n", entry_index, m_pagemap[entry_index].to_uint(), offset <<2, m_maincpu->pc());
			m_maincpu->set_mae();
			m_buserr[0] |= 0x80;    // invalid PTE
			m_buserr[0] &= ~0x8000; // read
			m_buserr[1] = offset<<2;
			if (mem_mask != ~0 && mem_mask != 0xffff0000 && mem_mask != 0xff000000)
			{
				if (mem_mask == 0x0000ffff || mem_mask == 0x0000ff00)
				{
					m_buserr[1] |= 2;
				}
				else if (mem_mask == 0x00ff0000)
				{
					m_buserr[1] |= 1;
				}
				else if (mem_mask == 0x000000ff)
				{
					m_buserr[1] |= 3;
				}
			}
		}
		return 0;
	}
}

WRITE32_MEMBER( sun4_state::sun4_insn_data_w )
{
	// it's translation time
	const uint32_t pmeg = m_curr_segmap_masked[(offset >> 16) & 0xfff];
	const uint32_t entry_index = pmeg | ((offset >> 11) & 0x1f);

	if (m_page_valid[entry_index])
	{
		page_entry_t &entry = m_pagemap[entry_index];
		entry.accessed = PM_ACCESSED;

		uint32_t tmp = entry.page | (offset & 0x7ff);

		//printf("sun4: write translated vaddr %08x to phys %08x type %d, PTE %08x, PC=%x\n", offset<<2, tmp<<2, entry.type, entry.to_uint(), m_maincpu->pc());

		switch (entry.type)
		{
		case 0: // type 0
			m_type0space->write32(tmp, data, mem_mask);
			return;

		case 1: // type 1
			//printf("write device space @ %x\n", tmp<<1);
			m_type1space->write32(tmp, data, mem_mask);
			return;

		default:
			//logerror("sun4: access to memory type not defined in sun4\n");
			return;
		}
	}
	else
	{
		//logerror("sun4: INVALID PTE entry %d %08x accessed! data=%08x vaddr=%x PC=%x\n", entry_index, m_pagemap[entry_index].to_uint(), data, offset <<2, m_maincpu->pc());
		//m_maincpu->trap(SPARC_DATA_ACCESS_EXCEPTION);
		//m_buserr[0] = 0x8;    // invalid PTE
		//m_buserr[1] = offset<<2;
	}
}

// make debugger fetches emulate supervisor program for best compatibility with boot PROM execution
//if (machine().side_effects_disabled()) asi = 9;

void sun4_state::l2p_command(int ref, const std::vector<std::string> &params)
{
	uint64_t addr, offset;

	if (!machine().debugger().commands().validate_number_parameter(params[0], addr)) return;

	addr &= 0xffffffff;
	offset = addr >> 2;

	uint8_t pmeg = 0;
	uint32_t entry_index = 0, tmp = 0;
	uint32_t entry_value = 0;

	if (m_arch == ARCH_SUN4)
	{
		pmeg = m_curr_segmap_masked[(offset >> 16) & 0xfff];
		entry_index = pmeg | ((offset >> 11) & 0x1f);
		tmp = m_pagemap[entry_index].page | (offset & 0x7ff);
		entry_value = m_pagemap[entry_index].to_uint();
	}

	if (m_page_valid[entry_index])
	{
		machine().debugger().console().printf("logical %08x => phys %08x, type %d (pmeg %d, entry %d PTE %08x)\n", addr, tmp << 2, m_pagemap[entry_index].type, pmeg, entry_index, entry_value);
	}
	else
	{
		machine().debugger().console().printf("logical %08x points to an invalid PTE! (pmeg %d, entry %d PTE %08x)\n", addr, tmp << 2, pmeg, entry_index, entry_value);
	}
}

void sun4_state::fcodes_command(int ref, const std::vector<std::string> &params)
{
#if SUN4_LOG_FCODES
	if (params < 1)
		return;

	bool is_on = (params[0] == "on");
	bool is_off = (params[0] == "off");

	if (!is_on && !is_off)
	{
		machine().debugger().console().printf("Please specify 'on' or 'off'.\n");
		return;
	}

	bool enabled = is_on;

	m_maincpu->enable_log_fcodes(enabled);
#endif
}

void sun4_state::sun4_system_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(sun4_state::sun4_system_r), FUNC(sun4_state::sun4_system_w));
}

void sun4_state::sun4_segment_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(sun4_state::sun4_segment_map_r), FUNC(sun4_state::sun4_segment_map_w));
}

void sun4_state::sun4_page_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(sun4_state::sun4_page_map_r), FUNC(sun4_state::sun4_page_map_w));
}

void sun4_state::sun4_supervisor_insn_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(sun4_state::sun4_insn_data_r<SUPER_INSN>), FUNC(sun4_state::sun4_insn_data_w));
}

void sun4_state::sun4_supervisor_data_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(sun4_state::sun4_insn_data_r<SUPER_DATA>), FUNC(sun4_state::sun4_insn_data_w));
}

void sun4_state::sun4_insn_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(sun4_state::sun4_insn_data_r<USER_INSN>), FUNC(sun4_state::sun4_insn_data_w));
}

void sun4_state::sun4_data_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(sun4_state::sun4_insn_data_r<USER_DATA>), FUNC(sun4_state::sun4_insn_data_w));
}

void sun4_state::sun4c_debugger_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(sun4_state::sun4c_debugger_r), FUNC(sun4_state::sun4c_debugger_w));
}

void sun4_state::type0space_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x00ffffff).rw(FUNC(sun4_state::ram_r), FUNC(sun4_state::ram_w));
	map(0x04000000, 0x0fffffff).rw(FUNC(sun4_state::timeout_r), FUNC(sun4_state::timeout_w));
}

void sun4_state::type1space_map(address_map &map)
{
	map(0x00000000, 0x0000000f).rw(m_scc1, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0xff00ff00);
	map(0x01000000, 0x0100000f).rw(m_scc2, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0xff00ff00);
	map(0x02000000, 0x020007ff).rw(m_timekpr, FUNC(timekeeper_device::read), FUNC(timekeeper_device::write));
	map(0x03000000, 0x0300000f).rw(FUNC(sun4_state::timer_r), FUNC(sun4_state::timer_w)).mirror(0xfffff0);
	map(0x05000000, 0x05000003).rw(FUNC(sun4_state::irq_r), FUNC(sun4_state::irq_w));
	map(0x06000000, 0x0607ffff).rom().region("user1", 0);
	map(0x07200000, 0x07200007).rw(FUNC(sun4_state::fdc_r), FUNC(sun4_state::fdc_w));
	map(0x07400003, 0x07400003).rw(FUNC(sun4_state::auxio_r), FUNC(sun4_state::auxio_w));
	map(0x08000000, 0x08000003).r(FUNC(sun4_state::ss1_sl0_id));    // slot 0 contains SCSI/DMA/Ethernet
	map(0x08400000, 0x0840000f).rw(FUNC(sun4_state::dma_r), FUNC(sun4_state::dma_w));
	map(0x08800000, 0x0880002f).m(m_scsi, FUNC(ncr53c90a_device::map)).umask32(0xff000000);
	map(0x08c00000, 0x08c00003).rw(m_lance, FUNC(am79c90_device::regs_r), FUNC(am79c90_device::regs_w));
}

void sun4_state::type1space_sbus_map(address_map &map)
{
	type1space_map(map);
	map(0x0a000000, 0x0fffffff).rw(m_sbus, FUNC(sbus_device::read), FUNC(sbus_device::write));
}

void sun4_state::type1space_s4_map(address_map &map)
{
	map(0x00000000, 0x0000000f).rw(m_scc1, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0xff00ff00);
	map(0x01000000, 0x0100000f).rw(m_scc2, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0xff00ff00);
}

/* Input ports */
static INPUT_PORTS_START( sun4 )
INPUT_PORTS_END

void sun4_state::machine_reset()
{
	m_auxio = 0xc0;
	m_irq_reg = 0;
	m_scc1_int = m_scc2_int = 0;
	m_scsi_irq = 0;
	m_fdc_irq = 0;
	m_dma_irq = false;
	m_dma_tc_read = false;
	m_dma_pack_register = 0;

	m_c0_last_read_time = attotime::zero;
	m_c1_last_read_time = attotime::zero;
	m_counter[0] = 1 << 10;
	m_counter[2] = 1 << 10;

	memset(m_counter, 0, sizeof(m_counter));
	memset(m_dma, 0, sizeof(m_dma));
}

void sun4_state::machine_start()
{
	m_rom_ptr = (uint32_t *)m_rom->base();
	m_ram_ptr = (uint32_t *)m_ram->pointer();
	m_ram_size = m_ram->size();
	m_ram_size_words = m_ram_size >> 2;

	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		using namespace std::placeholders;
		machine().debugger().console().register_command("l2p", CMDFLAG_NONE, 0, 1, 1, std::bind(&sun4_state::l2p_command, this, _1, _2));
		#if SUN4_LOG_FCODES
		machine().debugger().console().register_command("fcodes", CMDFLAG_NONE, 0, 1, 1, std::bind(&sun4_state::fcodes_command, this, _1, _2));
		#endif
	}

	// allocate timers for the built-in two channel timer
	m_c0_timer = timer_alloc(TIMER_0);
	m_c1_timer = timer_alloc(TIMER_1);
	m_c0_timer->adjust(attotime::from_usec(1), 0, attotime::from_usec(1));
	m_c1_timer->adjust(attotime::from_usec(1), 0, attotime::from_usec(1));

	// allocate timer for system reset
	m_reset_timer = timer_alloc(TIMER_RESET);
	m_reset_timer->adjust(attotime::never);

	for (int i = 0; i < 16; i++)
	{
		save_item(NAME(m_segmap[i]), i);
		save_item(NAME(m_segmap_masked[i]), i);
	}

	for (int i = 0; i < 16384; i++)
	{
		save_item(NAME(m_pagemap[i].valid), i);
		save_item(NAME(m_pagemap[i].writable), i);
		save_item(NAME(m_pagemap[i].supervisor), i);
		save_item(NAME(m_pagemap[i].uncached), i);
		save_item(NAME(m_pagemap[i].accessed), i);
		save_item(NAME(m_pagemap[i].modified), i);
		save_item(NAME(m_pagemap[i].page), i);
		save_item(NAME(m_pagemap[i].type), i);
	}

	save_item(NAME(m_cachetags));
	save_item(NAME(m_cachedata));
	save_item(NAME(m_ram_size));
	save_item(NAME(m_ram_size_words));
	save_item(NAME(m_context));
	save_item(NAME(m_context_masked));
	save_item(NAME(m_system_enable));
	save_item(NAME(m_fetch_bootrom));
	save_item(NAME(m_buserr));
	save_item(NAME(m_page_valid));

	save_item(NAME(m_auxio));
	save_item(NAME(m_counter));
	save_item(NAME(m_dma));
	save_item(NAME(m_dma_irq));
	save_item(NAME(m_dma_tc_read));
	save_item(NAME(m_dma_pack_register));
	save_item(NAME(m_scsi_irq));
	save_item(NAME(m_fdc_irq));

	save_item(NAME(m_irq_reg));
	save_item(NAME(m_scc1_int));
	save_item(NAME(m_scc2_int));
	save_item(NAME(m_diag));
	save_item(NAME(m_arch));
}

READ32_MEMBER( sun4_state::ram_r )
{
	return m_ram_ptr[offset];
}

WRITE32_MEMBER( sun4_state::ram_w )
{
#if 0
	// if writing bad parity is enabled
	if (((m_parregs[0] & 0x20000000) == 0x20000000) &&
		(m_irqctrl & 0x01000000) &&
		!(m_bInBusErr))
	{
		m_parregs[1] = offset<<2;
		//printf("Generating parity error, mem_mask %08x\n", mem_mask);
		switch (mem_mask)
		{
			case 0xff000000:
				m_parregs[0] |= 0x08<<24;
				break;

			case 0x00ff0000:
				m_parregs[1] += 1;
				m_parregs[0] |= 0x04<<24;
				break;

			case 0x0000ff00:
				m_parregs[1] += 2;
				m_parregs[0] |= 0x02<<24;
				break;

			case 0x000000ff:
				m_parregs[1] += 3;
				m_parregs[0] |= 0x01<<24;
				break;

			case 0x0000ffff:
				m_parregs[1] += 2;
				m_parregs[0] |= 0x03<<24;
				break;

			case 0xffff0000:
				m_parregs[0] |= 0x0c<<24;
				break;

			case 0xffffffff:    // no address adjust, show all 4 lanes as problematic
				m_parregs[0] |= 0x0f<<24;
				break;
		}

		// indicate parity interrupt
		m_parregs[0] |= 0x80000000;

		// and can we take that now?
		if (m_parregs[0] & 0x40000000)
		{
		}
	}
#endif

	//printf("ram_w: %08x to %08x (mask %08x)\n", data, offset<<2, mem_mask);

	//if ((offset<<2) == 0xfb2000) printf("write %08x to %08x, mask %08x, PC=%x\n", data, offset<<2, mem_mask, m_maincpu->pc());

	COMBINE_DATA(&m_ram_ptr[offset]);
}

READ32_MEMBER( sun4_state::timeout_r )
{
	m_buserr[0] = 0x20; // read timeout
	m_buserr[1] = 0x04000000 + (offset << 2);
	m_maincpu->set_mae();
	return 0;
}

WRITE32_MEMBER( sun4_state::timeout_w )
{
	m_buserr[0] = 0x8020; // write timeout
	m_buserr[1] = 0x04000000 + (offset << 2);
	m_maincpu->set_mae();
}

READ8_MEMBER( sun4_state::fdc_r )
{
	if (machine().side_effects_disabled())
		return 0;

	switch(offset)
	{
		case 0: // Main Status (R, 82072)
			return m_fdc->msr_r();

		case 1: // FIFO Data Port (R, 82072)
		case 5: // FIFO Data Port (R, 82077)
			return m_fdc->fifo_r();

		case 2: // Digital Output Register (R, 82077)
			return m_fdc->dor_r();

		case 4: // Main Status Register (R, 82077)
			return m_fdc->msr_r();

		case 7:// Digital Input Register (R, 82077)
			return m_fdc->dir_r();

		default:
			break;
	}

	return 0;
}

WRITE8_MEMBER( sun4_state::fdc_w )
{
	switch(offset)
	{
		case 0: // Data Rate Select Register (W, 82072)
		case 4: // Data Rate Select Register (W, 82077)
			m_fdc->dsr_w(data);
			break;

		case 1: // FIFO Data Port (W, 82072)
		case 5: // FIFO Data Port (W, 82077)
			m_fdc->fifo_w(data);
			break;

		case 7: // Configuration Control REgister (W, 82077)
			m_fdc->ccr_w(data);
			break;

		default:
			break;
	}
}

READ8_MEMBER( sun4_state::auxio_r )
{
	//logerror("%s: auxio_r: %02x\n", machine().describe_context(), m_auxio);
	return m_auxio;
}

WRITE8_MEMBER( sun4_state::auxio_w )
{
	//logerror("%s: auxio_w: %02x, drive_sel:%d tc:%d eject:%d LED:%d\n", machine().describe_context(), data, BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));
	m_auxio = (m_auxio & 0xf0) | (data & 0x0f);
	if (!(m_auxio & AUXIO_DRIVE_SEL))
	{
		m_auxio &= ~(AUXIO_DENSITY | AUXIO_DISK_CHG);
	}
	else
	{
		m_auxio |= AUXIO_DISK_CHG; // Report no disk inserted
		m_fdc->tc_w(data & AUXIO_TC);
	}
}

READ8_MEMBER( sun4_state::irq_r )
{
	//logerror("%02x from IRQ\n", m_irq_reg);
	return m_irq_reg;
}

WRITE8_MEMBER( sun4_state::irq_w )
{
	const uint8_t old_irq = m_irq_reg;
	m_irq_reg = data;
	const uint8_t changed = old_irq ^ data;

	//logerror("%02x to IRQ, %02x changed\n", data, changed);

	if (!changed)
		return;

	if (BIT(changed, 0))
	{
		if (BIT(m_irq_reg, 7) && BIT(m_counter[2] | m_counter[3], 31))
			m_maincpu->set_input_line(SPARC_IRQ14, BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
		if (BIT(m_irq_reg, 5) && BIT(m_counter[0] | m_counter[1], 31))
			m_maincpu->set_input_line(SPARC_IRQ10, BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
		if (BIT(m_irq_reg, 3))
			m_maincpu->set_input_line(SPARC_IRQ6, BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
		if (BIT(m_irq_reg, 2))
			m_maincpu->set_input_line(SPARC_IRQ4, BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
		if (BIT(m_irq_reg, 1))
			m_maincpu->set_input_line(SPARC_IRQ1, BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
	}
	else if (BIT(m_irq_reg, 0))
	{
		if (BIT(changed, 7) && BIT(m_counter[2] | m_counter[3], 31))
			m_maincpu->set_input_line(SPARC_IRQ14, BIT(m_irq_reg, 7) ? ASSERT_LINE : CLEAR_LINE);
		if (BIT(changed, 5) && BIT(m_counter[0] | m_counter[1], 31))
			m_maincpu->set_input_line(SPARC_IRQ10, BIT(m_irq_reg, 5) ? ASSERT_LINE : CLEAR_LINE);
		if (BIT(changed, 3))
			m_maincpu->set_input_line(SPARC_IRQ6, BIT(m_irq_reg, 3) ? ASSERT_LINE : CLEAR_LINE);
		if (BIT(changed, 2))
			m_maincpu->set_input_line(SPARC_IRQ4, BIT(m_irq_reg, 2) ? ASSERT_LINE : CLEAR_LINE);
		if (BIT(changed, 1))
			m_maincpu->set_input_line(SPARC_IRQ1, BIT(m_irq_reg, 1) ? ASSERT_LINE : CLEAR_LINE);
	}
}

WRITE_LINE_MEMBER( sun4_state::scc1_int )
{
	m_scc1_int = state;

	m_maincpu->set_input_line(SPARC_IRQ12, ((m_scc1_int || m_scc2_int) && (m_irq_reg & 0x01)) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( sun4_state::scc2_int )
{
	m_scc2_int = state;

	m_maincpu->set_input_line(SPARC_IRQ12, ((m_scc1_int || m_scc2_int) && (m_irq_reg & 0x01)) ? ASSERT_LINE : CLEAR_LINE);
}

void sun4_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_0:
			//logerror("Timer 0 expired\n");
			m_counter[0] += 1 << 10;
			if ((m_counter[0] & 0x7fffffff) == (m_counter[1] & 0x7fffffff))
			{
				m_counter[0] = 0x80000000 | (1 << 10);
				m_counter[1] |= 0x80000000;
				if ((m_irq_reg & 0x21) == 0x21)
				{
					m_maincpu->set_input_line(SPARC_IRQ10, ASSERT_LINE);
					//logerror("Taking INT10\n");
				}
				else
				{
					//logerror("Not taking INT10\n");
				}
			}
			break;

		case TIMER_1:
			//logerror("Timer 1 expired\n");
			m_counter[2] += 1 << 10;
			if ((m_counter[2] & 0x7fffffff) == (m_counter[3] & 0x7fffffff))
			{
				m_counter[2] = 0x80000000 | (1 << 10);
				m_counter[3] |= 0x80000000;
				if ((m_irq_reg & 0x81) == 0x81)
				{
					m_maincpu->set_input_line(SPARC_IRQ14, ASSERT_LINE);
					//logerror("Taking INT14\n");
				}
				else
				{
					//logerror("Not taking INT14\n");
				}
			}
			break;

		case TIMER_RESET:
			m_reset_timer->adjust(attotime::never);
			m_maincpu->set_input_line(SPARC_RESET, CLEAR_LINE);
			//printf("Clearing reset line\n");
			break;
	}
}

READ32_MEMBER( sun4_state::timer_r )
{
	const uint32_t ret = m_counter[offset];

	// reading limt 0
	if (offset == 0)
	{
		//logerror("Read timer counter 0 (%x) @ %x, mask %08x\n", ret, m_maincpu->pc(), mem_mask);
	}
	else if (offset == 1)
	{
		//logerror("Read timer limit 0 (%08x) @ %x, mask %08x, clearing IRQ10\n", ret, m_maincpu->pc(), mem_mask);
		m_counter[0] &= ~0x80000000;
		m_counter[1] &= ~0x80000000;
		m_maincpu->set_input_line(SPARC_IRQ10, CLEAR_LINE);
	}
	else if (offset == 2)
	{
		//logerror("Read timer counter 1 (%x) @ %x, mask %08x\n", ret, m_maincpu->pc(), mem_mask);
	}
	else if (offset == 3)
	{
		//logerror("Read timer limit 1 (%08x) @ %x, mask %08x, clearing IRQ14\n", ret, m_maincpu->pc(), mem_mask);
		m_counter[2] &= ~0x80000000;
		m_counter[3] &= ~0x80000000;
		m_maincpu->set_input_line(SPARC_IRQ14, CLEAR_LINE);
	}

	return ret;
}

WRITE32_MEMBER( sun4_state::timer_w )
{
	COMBINE_DATA(&m_counter[offset]);

	if (offset == 0)
	{
		//logerror("%08x to timer counter 0 @ %x, mask %08x\n", data, m_maincpu->pc(), mem_mask);
	}

	// writing limit 0
	if (offset == 1)
	{
		//logerror("%08x to timer limit 0 @ %x, mask %08x\n", data, m_maincpu->pc(), mem_mask);
		m_counter[0] = 1 << 10;
	}

	if (offset == 2)
	{
		//printf("%08x to timer counter 1 @ %x, mask %08x\n", data, m_maincpu->pc(), mem_mask);
	}

	// writing limit 1
	if (offset == 3)
	{
		//logerror("%08x to timer limit 1 @ %x, mask %08x\n", data, m_maincpu->pc(), mem_mask);
		m_counter[2] = 1 << 10;
	}
}

void sun4_state::dma_check_interrupts()
{
	bool tc_interrupt = (m_dma[DMA_CTRL] & DMA_TC) != 0 && !m_dma_tc_read;
	bool scsi_interrupt = m_scsi_irq != 0;

	m_dma[DMA_CTRL] &= ~DMA_INT_PEND;
	if (tc_interrupt || scsi_interrupt)
		m_dma[DMA_CTRL] |= DMA_INT_PEND;

	int irq_or_err_pending = (m_dma[DMA_CTRL] & (DMA_INT_PEND | DMA_ERR_PEND)) ? 1 : 0;
	int irq_enabled = (m_dma[DMA_CTRL] & DMA_INT_EN) ? 1 : 0;
	bool old_irq = m_dma_irq;
	m_dma_irq = irq_or_err_pending && irq_enabled;
	if (old_irq != m_dma_irq)
	{
		//logerror("m_dma_irq %d because irq_or_err_pending:%d and irq_enabled:%d\n", m_dma_irq ? 1 : 0, irq_or_err_pending, irq_enabled);
		m_maincpu->set_input_line(SPARC_IRQ3, m_dma_irq ? ASSERT_LINE : CLEAR_LINE);
	}
}

void sun4_state::dma_transfer_write()
{
	//logerror("DMAing from device to RAM\n");

	uint8_t pack_cnt = (m_dma[DMA_CTRL] & DMA_PACK_CNT) >> DMA_PACK_CNT_SHIFT;
	while (m_dma[DMA_CTRL] & DMA_REQ_PEND)
	{
		int bit_index = (3 - pack_cnt) * 8;
		uint8_t dma_value = m_scsi->dma_r();
		//logerror("Read from device: %02x, pack count %d\n", dma_value, pack_cnt);
		m_dma_pack_register |= dma_value << bit_index;

		if (m_dma[DMA_CTRL] & DMA_EN_CNT)
			m_dma[DMA_BYTE_COUNT]--;

		pack_cnt++;
		if (pack_cnt == 4)
		{
			//logerror("Writing pack register %08x to %08x\n", m_dma_pack_register, m_dma[DMA_ADDR]);
			if (m_arch == ARCH_SUN4C)
				m_mmu->insn_data_w<sun4c_mmu_device::SUPER_DATA>(m_dma[DMA_ADDR] >> 2, m_dma_pack_register, ~0);
			//else - TODO
				//sun4_insn_data_w(space, m_dma[DMA_ADDR] >> 2, m_dma_pack_register, ~0);

			pack_cnt = 0;
			m_dma_pack_register = 0;
			m_dma[DMA_ADDR] += 4;
		}
	}

	m_dma[DMA_CTRL] &= ~DMA_PACK_CNT;
	m_dma[DMA_CTRL] |= pack_cnt << DMA_PACK_CNT_SHIFT;
}

void sun4_state::dma_transfer_read()
{
	//logerror("DMAing from RAM to device\n");

	bool word_cached = false;
	uint32_t current_word = 0;
	while (m_dma[DMA_CTRL] & DMA_REQ_PEND)
	{
		if (!word_cached)
		{
			if (m_arch == ARCH_SUN4C)
				current_word = m_mmu->insn_data_r<sun4c_mmu_device::SUPER_DATA>(m_dma[DMA_ADDR] >> 2, ~0);
			//else - TODO
				//current_word = sun4_insn_data_r<SUPER_DATA>(space, m_dma[DMA_ADDR] >> 2, ~0);
			word_cached = true;
			//logerror("Current word: %08x\n", current_word);
		}

		int bit_index = (3 - (m_dma[DMA_ADDR] & 3)) * 8;
		uint8_t dma_value(current_word >> bit_index);
		//logerror("Write to device: %02x\n", dma_value);
		m_scsi->dma_w(dma_value);

		if ((m_dma[DMA_ADDR] & 3) == 3)
			word_cached = false;

		m_dma[DMA_ADDR]++;
		if (m_dma[DMA_CTRL] & DMA_EN_CNT)
			m_dma[DMA_BYTE_COUNT]--;
	}
}

void sun4_state::dma_transfer()
{
	if (m_dma[DMA_CTRL] & DMA_WRITE)
		dma_transfer_write();
	else
		dma_transfer_read();

	if (m_dma[DMA_BYTE_COUNT] == 0 && (m_dma[DMA_CTRL] & DMA_EN_CNT))
	{
		m_dma[DMA_CTRL] |= DMA_TC;
		m_dma_tc_read = false;
		dma_check_interrupts();
	}
}

READ32_MEMBER( sun4_state::dma_r )
{
	if (offset == DMA_CTRL && (m_dma[DMA_CTRL] & DMA_TC) != 0)
	{
		m_dma_tc_read = true;
		dma_check_interrupts();
	}
	return m_dma[offset];
}

WRITE32_MEMBER( sun4_state::dma_w )
{
	switch (offset)
	{
		case DMA_CTRL:
		{
			// clear write-only bits
			//logerror("dma_w: ctrl: %08x\n", data);

			mem_mask &= (DMA_READ_WRITE);
			COMBINE_DATA(&m_dma[DMA_CTRL]);

			if (data & DMA_RESET)
			{
				//logerror("dma_w: reset\n");
				m_dma[DMA_CTRL] &= ~(DMA_ERR_PEND | DMA_PACK_CNT | DMA_INT_EN | DMA_FLUSH | DMA_DRAIN | DMA_WRITE | DMA_EN_DMA | DMA_REQ_PEND | DMA_EN_CNT | DMA_TC);
			}
			else if (data & DMA_FLUSH)
			{
				//logerror("dma_w: flush\n");
				m_dma[DMA_CTRL] &= ~(DMA_PACK_CNT | DMA_ERR_PEND | DMA_TC);

				dma_check_interrupts();
			}
			else if (data & DMA_DRAIN)
			{
				m_dma[DMA_CTRL] &= ~DMA_DRAIN;
				const uint8_t pack_cnt = (m_dma[DMA_CTRL] & DMA_PACK_CNT) >> DMA_PACK_CNT_SHIFT;
				for (uint8_t i = 0; i < pack_cnt; i++)
				{
					const uint32_t bit_index = (3 - i) * 8;
					//const uint32_t value = m_dma_pack_register & (0xff << bit_index);
					//logerror("dma_w: draining %02x to RAM address %08x & %08x\n", value >> bit_index, m_dma[DMA_ADDR], 0xff << (24 - bit_index));
					if (m_arch == ARCH_SUN4C)
						m_mmu->insn_data_w<sun4c_mmu_device::SUPER_DATA>(m_dma[DMA_ADDR] >> 2, m_dma_pack_register, 0xff << bit_index);
					else
						sun4_insn_data_w(space, m_dma[DMA_ADDR] >> 2, m_dma_pack_register, 0xff << bit_index);
					m_dma[DMA_ADDR]++;
				}
				m_dma_pack_register = 0;
				m_dma[DMA_CTRL] &= ~DMA_PACK_CNT;
			}
			// TODO: DMA_DRAIN

			if (data & DMA_EN_DMA && (m_dma[DMA_CTRL] & DMA_REQ_PEND))
				dma_transfer();
			break;
		}

		case DMA_ADDR:
			//logerror("dma_w: addr: %08x\n", data);
			m_dma[offset] = data;
			break;

		case DMA_BYTE_COUNT:
			//logerror("dma_w: byte_count: %08x\n", data);
			m_dma[offset] = data;
			break;

		default:
			break;
	}
}

WRITE_LINE_MEMBER( sun4_state::scsi_irq )
{
	int old_irq = m_scsi_irq;
	m_scsi_irq = state;
	if (m_scsi_irq != old_irq)
	{
		//logerror("scsi_irq %d, checking interrupts\n", state);
		dma_check_interrupts();
	}
}

WRITE_LINE_MEMBER( sun4_state::scsi_drq )
{
	//logerror("scsi_drq %d\n", state);
	m_dma[DMA_CTRL] &= ~DMA_REQ_PEND;
	if (state)
	{
		//logerror("scsi_drq, DMA pending\n");
		m_dma[DMA_CTRL] |= DMA_REQ_PEND;
		if (m_dma[DMA_CTRL] & DMA_EN_DMA/* && m_dma[DMA_BYTE_COUNT]*/)
		{
			//logerror("DMA enabled, starting dma\n");
			dma_transfer();
		}
	}
}

WRITE_LINE_MEMBER( sun4_state::fdc_irq )
{
	int old_irq = m_fdc_irq;
	m_fdc_irq = state;
	if (old_irq != m_fdc_irq)
	{
		logerror("fdc_irq %d\n", state);
		m_maincpu->set_input_line(SPARC_IRQ11, state ? ASSERT_LINE : CLEAR_LINE);
	}
}

WRITE32_MEMBER( sun4_state::buserr_w )
{
	COMBINE_DATA(&m_buserr[offset]);
}

// indicate 4/60 SCSI/DMA/Ethernet card exists
READ32_MEMBER( sun4_state::ss1_sl0_id )
{
	return 0xfe810101;
}

FLOPPY_FORMATS_MEMBER( sun4_state::floppy_formats )
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static void sun_floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
}

static void sun4_cdrom(device_t *device)
{
	downcast<nscsi_cdrom_device &>(*device).set_block_size(512);
}

static void sun_scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add_internal("ncr53c90a", NCR53C90A);
	device.set_option_machine_config("cdrom", sun4_cdrom);
}

void sun4_state::ncr53c90a(device_t *device)
{
	ncr53c90a_device &adapter = downcast<ncr53c90a_device &>(*device);

	adapter.set_clock(10000000);
	adapter.irq_handler_cb().set(*this, FUNC(sun4_state::scsi_irq));
	adapter.drq_handler_cb().set(*this, FUNC(sun4_state::scsi_drq));
}

void sun4_state::sun4(machine_config &config)
{
	/* basic machine hardware */
	MB86901(config, m_maincpu, 16'670'000);
	m_maincpu->add_asi_desc([](sparc_disassembler *dasm) { dasm->add_asi_desc(sun4_asi_desc); });
	m_maincpu->set_addrmap(0, &sun4_state::sun4c_debugger_map);

	// TODO: MMU for sun4 hardware
	SUN4C_MMU(config, m_mmu, 25'000'000, 7, 0x7f);
	m_mmu->type1_r().set(m_type1space, FUNC(address_map_bank_device::read32));
	m_mmu->type1_w().set(m_type1space, FUNC(address_map_bank_device::write32));
	m_mmu->set_cpu(m_maincpu);
	m_mmu->set_ram(m_ram);
	m_mmu->set_rom("user1");
	m_mmu->set_scc(m_scc2);
	m_maincpu->set_mmu(m_mmu);

	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	m_ram->set_default_value(0x00);

	M48T02(config, TIMEKEEPER_TAG, 0);

	I82072(config, m_fdc, 24_MHz_XTAL);
	m_fdc->set_ready_line_connected(false);
	m_fdc->intrq_wr_callback().set(FUNC(sun4_state::fdc_irq));
	FLOPPY_CONNECTOR(config, m_floppy, sun_floppies, "35hd", sun4_state::floppy_formats);

	// MMU Type 0 device space
	ADDRESS_MAP_BANK(config, m_type0space).set_map(&sun4_state::type0space_map).set_options(ENDIANNESS_BIG, 32, 32, 0x80000000);

	// MMU Type 1 device space
	ADDRESS_MAP_BANK(config, m_type1space).set_map(&sun4_state::type1space_s4_map).set_options(ENDIANNESS_BIG, 32, 32, 0x80000000);

	// Ethernet
	AM79C90(config, m_lance);
	m_lance->dma_in().set([this](address_space &space, offs_t offset)
	{
		u32 const data = sun4_insn_data_r<SUPER_DATA>(space, (0xff000000U | offset) >> 2);

		return (offset & 2) ? u16(data) : u16(data >> 16);
	});
	m_lance->dma_out().set([this](address_space &space, offs_t offset, u16 data, u16 mem_mask)
	{
		if (offset & 2)
			sun4_insn_data_w(space, (0xff000000U | offset) >> 2, data, mem_mask);
		else
			sun4_insn_data_w(space, (0xff000000U | offset) >> 2, u32(data) << 16, u32(mem_mask) << 16);
	});

	// Keyboard/mouse
	SCC8530N(config, m_scc1, 4.9152_MHz_XTAL);
	m_scc1->out_int_callback().set(FUNC(sun4_state::scc1_int));
	m_scc1->out_txda_callback().set(KEYBOARD_TAG, FUNC(sun_keyboard_port_device::write_txd));
	m_scc1->out_txdb_callback().set(MOUSE_TAG, FUNC(sun_mouse_port_device::write_txd));

	SUNKBD_PORT(config, KEYBOARD_TAG, default_sun_keyboard_devices, "type4hle").rxd_handler().set(m_scc1, FUNC(z80scc_device::rxa_w));
	SUNMOUSE_PORT(config, MOUSE_TAG, default_sun_mouse_devices, "hle1200").rxd_handler().set(m_scc1, FUNC(z80scc_device::rxb_w));

	// RS232 serial ports
	SCC8530N(config, m_scc2, 4.9152_MHz_XTAL);
	m_scc2->out_int_callback().set(FUNC(sun4_state::scc2_int));
	m_scc2->out_txda_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_txd));
	m_scc2->out_txdb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, RS232A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc2, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc2, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc2, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc2, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc2, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc2, FUNC(z80scc_device::ctsb_w));

	NSCSI_BUS(config, "scsibus");
	NSCSI_CONNECTOR(config, "scsibus:0", sun_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsibus:1", sun_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:2", sun_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:3", sun_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:4", sun_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:5", sun_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:6", sun_scsi_devices, "cdrom");
	NSCSI_CONNECTOR(config, "scsibus:7", sun_scsi_devices, "ncr53c90a", true).set_option_machine_config("ncr53c90a", [this] (device_t *device) { ncr53c90a(device); });
}

void sun4_state::sun4c(machine_config &config)
{
	/* basic machine hardware */
	MB86901(config, m_maincpu, 20'000'000);
	m_maincpu->add_asi_desc([](sparc_disassembler *dasm) { dasm->add_asi_desc(sun4c_asi_desc); });
	m_maincpu->set_addrmap(0, &sun4_state::sun4c_debugger_map);

	SUN4C_MMU(config, m_mmu, 20'000'000, 7, 0x7f);
	m_mmu->type1_r().set(m_type1space, FUNC(address_map_bank_device::read32));
	m_mmu->type1_w().set(m_type1space, FUNC(address_map_bank_device::write32));
	m_mmu->set_cpu(m_maincpu);
	m_mmu->set_ram(m_ram);
	m_mmu->set_rom("user1");
	m_mmu->set_scc(m_scc2);
	m_maincpu->set_mmu(m_mmu);

	RAM(config, m_ram).set_default_size("16M").set_default_value(0x00);
	m_ram->set_extra_options("4M,8M,12M,16M,20M,24M,28M,32M,36M,40M,48M,52M,64M");

	M48T02(config, TIMEKEEPER_TAG, 0);

	N82077AA(config, m_fdc, 24_MHz_XTAL);
	m_fdc->set_ready_line_connected(false);
	m_fdc->intrq_wr_callback().set(FUNC(sun4_state::fdc_irq));
	FLOPPY_CONNECTOR(config, m_floppy, sun_floppies, "35hd", sun4_state::floppy_formats);

	// MMU Type 0 device space
	ADDRESS_MAP_BANK(config, m_type0space).set_map(&sun4_state::type0space_map).set_options(ENDIANNESS_BIG, 32, 32, 0x80000000);

	// MMU Type 1 device space
	ADDRESS_MAP_BANK(config, m_type1space).set_map(&sun4_state::type1space_sbus_map).set_options(ENDIANNESS_BIG, 32, 32, 0x80000000);

	// Ethernet
	AM79C90(config, m_lance);
	m_lance->dma_in().set([this](offs_t offset)
	{
		u32 const data = m_mmu->insn_data_r<sun4c_mmu_device::SUPER_DATA>((0xff000000U | offset) >> 2, 0xffffffffU);

		return (offset & 2) ? u16(data) : u16(data >> 16);
	});
	m_lance->dma_out().set([this](offs_t offset, u16 data, u16 mem_mask)
	{
		if (offset & 2)
			m_mmu->insn_data_w<sun4c_mmu_device::SUPER_DATA>((0xff000000U | offset) >> 2, data, mem_mask);
		else
			m_mmu->insn_data_w<sun4c_mmu_device::SUPER_DATA>((0xff000000U | offset) >> 2, u32(data) << 16, u32(mem_mask) << 16);
	});

	// Keyboard/mouse
	SCC8530N(config, m_scc1, 4.9152_MHz_XTAL);
	m_scc1->out_int_callback().set(FUNC(sun4_state::scc1_int));
	m_scc1->out_txda_callback().set(KEYBOARD_TAG, FUNC(sun_keyboard_port_device::write_txd));
	// no mouse TxD connection - replaced with soft power request input

	SUNKBD_PORT(config, KEYBOARD_TAG, default_sun_keyboard_devices, "type5hle").rxd_handler().set(m_scc1, FUNC(z80scc_device::rxa_w));
	SUNMOUSE_PORT(config, MOUSE_TAG, default_sun_mouse_devices, "hle1200").rxd_handler().set(m_scc1, FUNC(z80scc_device::rxb_w));

	// RS232 serial ports
	SCC8530N(config, m_scc2, 4.9152_MHz_XTAL);
	m_scc2->out_int_callback().set(FUNC(sun4_state::scc2_int));
	m_scc2->out_txda_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_txd));
	m_scc2->out_txdb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, RS232A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc2, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc2, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc2, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc2, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc2, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc2, FUNC(z80scc_device::ctsb_w));

	NSCSI_BUS(config, "scsibus");
	NSCSI_CONNECTOR(config, "scsibus:0", sun_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsibus:1", sun_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:2", sun_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:3", sun_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:4", sun_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:5", sun_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:6", sun_scsi_devices, "cdrom");
	NSCSI_CONNECTOR(config, "scsibus:7", sun_scsi_devices, "ncr53c90a", true).set_option_machine_config("ncr53c90a", [this] (device_t *device) { ncr53c90a(device); });

	// SBus
	SBUS(config, m_sbus, 20'000'000, "maincpu", "type1");
	SBUS_SLOT(config, m_sbus_slot[0], 20'000'000, m_sbus, 0, sbus_cards, nullptr);
	SBUS_SLOT(config, m_sbus_slot[1], 20'000'000, m_sbus, 1, sbus_cards, nullptr);
	SBUS_SLOT(config, m_sbus_slot[2], 20'000'000, m_sbus, 2, sbus_cards, nullptr);
}

void sun4_state::sun4_20(machine_config &config)
{
	sun4c(config);

	m_ram->set_extra_options("4M,8M,12M,16M");

	m_sbus_slot[0]->set_fixed(true);
	m_sbus_slot[1]->set_fixed(true);
	m_sbus_slot[2]->set_default_option("bwtwo");
	m_sbus_slot[2]->set_fixed(true);
}

void sun4_state::sun4_40(machine_config &config)
{
	sun4c(config);

	m_ram->set_extra_options("4M,8M,12M,16M,20M,24M,32M,36M,48M");

	m_mmu->set_clock(25'000'000);
	m_maincpu->set_clock(25'000'000);

	m_sbus->set_clock(25'000'000);
	m_sbus_slot[0]->set_clock(25'000'000);
	m_sbus_slot[1]->set_clock(25'000'000);
	m_sbus_slot[2]->set_clock(25'000'000);
	m_sbus_slot[2]->set_default_option("bwtwo");
	m_sbus_slot[2]->set_fixed(true);
}

void sun4_state::sun4_50(machine_config &config)
{
	sun4c(config);

	m_mmu->set_ctx_mask(0xf);
	m_mmu->set_pmeg_mask(0xff);

	m_mmu->set_clock(40'000'000);
	m_maincpu->set_clock(40'000'000);

	m_sbus->set_clock(20'000'000);
	m_sbus_slot[0]->set_clock(20'000'000);
	m_sbus_slot[1]->set_clock(20'000'000);
	m_sbus_slot[2]->set_clock(20'000'000);
	m_sbus_slot[2]->set_default_option("turbogx"); // not accurate, should be gxp, not turbogx
	m_sbus_slot[2]->set_fixed(true);
}

void sun4_state::sun4_60(machine_config &config)
{
	sun4c(config);
}

void sun4_state::sun4_65(machine_config &config)
{
	sun4c(config);

	m_mmu->set_clock(25'000'000);
	m_maincpu->set_clock(25'000'000);

	m_sbus->set_clock(25'000'000);
	m_sbus_slot[0]->set_clock(25'000'000);
	m_sbus_slot[1]->set_clock(25'000'000);
	m_sbus_slot[2]->set_clock(25'000'000);
	m_sbus_slot[2]->set_default_option("bwtwo");
}

void sun4_state::sun4_75(machine_config &config)
{
	sun4c(config);

	m_mmu->set_ctx_mask(0xf);
	m_mmu->set_pmeg_mask(0xff);

	m_mmu->set_clock(40'000'000);
	m_maincpu->set_clock(40'000'000);
}

/*
Boot PROM

Sun-4c Architecture

SPARCstation SLC (Sun-4/20) - 128K x 8
U1001       Revision
========================================
520-2748-01 1.2 Version 3
520-2748-02 1.3
520-2748-03 1.3
520-2748-04 1.4 Version 2
595-2250-xx Sun-4/20 Boot PROM Kit

SPARCstation ELC (Sun-4/25) - 256K x 8
U0806       Revision
========================================
520-3085-01
520-3085-02 2.3 Version 95
520-3085-03 2.4 Version 96
520-3085-04 2.6 Version 102 (not used)
520-3085-04 2.9 Version 7


SPARCstation IPC (Sun-4/40) - 256K x 8
U0902       Revision
========================================
525-1085-01
525-1085-02
525-1085-03 1.6 Version 151
525-1191-01 1.7 Version 3 and 2.4 Version 362
525-1191-02 1.7 Version 3 and 2.6 Version 411
525-1191-03 1.7 Version 3 and 2.9 Version 24


SPARCstation IPX (Sun-4/50) - 256K x 8
U0501       Revision
========================================
525-1177-01 2.1 Version 66
525-1177-02 2.2 Version 134
525-1177-03 2.3 Version 263
525-1177-04 2.4 Version 347
525-1177-05 2.6 Version 410
525-1177-06 2.9 Version 20


SPARCstation 1 (Sun-4/60) - 128K x 8
U0837       Revision
========================================
525-1043-01
525-1043-02 0.1
525-1043-03
525-1043-04 1.0
525-1043-05 1.0
525-1043-06 1.0
525-1043-07 1.1
525-1043-08 1.3 Version 3
595-1963-xx Sun-4/60 Boot PROM Kit
525-1207-01 2.4 Version 95
525-1207-02 2.9 Version 9 (2.x is only available from the spare parts price list)
560-1805-xx Sun-4/60 2.4 Boot PROM Kit


SPARCstation 1+ (Sun-4/65) - 128K x 8
U0837 Revision
========================================
525-1108-01
525-1108-02
525-1108-03 1.1 Version 13
525-1108-04 1.2
525-1108-05 1.3 Version 4
525-1208-01 2.4 Version 116
525-1208-02 2.9 Version 9 (2.x is only available from the spare parts price list)
560-1806-xx Sun-4/65 2.4 Boot PROM Kit


SPARCstation 2 (Sun-4/75) - 256K x 8
U0501       Revision
========================================
525-1107-01 2.0Beta0
525-1107-02 2.0Beta1
525-1107-03 2.0
525-1107-04 2.0 Version 865 (fails with Weitek Power ?p)
525-1107-05 2.1 Version 931 (fails with Weitek Power ?p)
525-1107-06 2.2 Version 947
525-1107-07 2.4 Version 990
525-1107-08 2.4.1 Version 991
525-1107-09 2.6 Version 1118
525-1107-10 2.9 Version 16
595-2249-xx Sun-4/75 Boot PROM Kit

*/

ROM_START( sun4_110 )
	ROM_REGION32_BE( 0x80000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD32_BYTE( "520-1651-09_2.8.1.bin", 0x000003, 0x010000, CRC(9b439222) SHA1(b3589f65478e53338aee6355567484421a913d00) )
	ROM_LOAD32_BYTE( "520-1652-09_2.8.1.bin", 0x000002, 0x010000, CRC(2bed25ec) SHA1(a9ff6c94ec8e0d6b084a300ff7bd8f2126c7a3b1) )
	ROM_LOAD32_BYTE( "520-1653-09_2.8.1.bin", 0x000001, 0x010000, CRC(d44b7f76) SHA1(2acea449d7782a10fda7f6529279a7e1882549e3) )
	ROM_LOAD32_BYTE( "520-1654-09_2.8.1.bin", 0x000000, 0x010000, CRC(1bef8469) SHA1(d5a89d29df7ffc01b305cd12d0b6eb77e126dcbf) )
ROM_END

// Sun 4/300, Cypress Semiconductor CY7C601, Texas Instruments 8847 FPU
ROM_START( sun4_300 )
	ROM_REGION32_BE( 0x80000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD32_BYTE( "1035-09.rom", 0x00003, 0x10000, CRC(4ae2f2ad) SHA1(9c17a80b3ce3efdf18b5eca969f1565ddaad3116))
	ROM_LOAD32_BYTE( "1036-09.rom", 0x00000, 0x10000, CRC(cb3d45a7) SHA1(9d5da09ff87ec52dc99ffabd1003d30811eafdb0))
	ROM_LOAD32_BYTE( "1037-09.rom", 0x00001, 0x10000, CRC(4f005bea) SHA1(db3f6133ea7c497ba440bc797123dde41abea6fd))
	ROM_LOAD32_BYTE( "1038-09.rom", 0x00002, 0x10000, CRC(1e429d31) SHA1(498ce4d34a74ea6e3e369bb7eb9c2b87e12bd080))
ROM_END

ROM_START( sun4_400 )
	ROM_REGION32_BE( 0x80000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD32_BYTE( "525-1103-06_4.1.1.bin", 0x000000, 0x010000, CRC(c129c0a8) SHA1(4ecd51fb924e65f773a09cae35ce16b1744bd7b9) )
	ROM_LOAD32_BYTE( "525-1104-06_4.1.1.bin", 0x000001, 0x010000, CRC(fe3a95fc) SHA1(c3ebb89eb07d421ed4f3d7e1a66eb286f5a743e9) )
	ROM_LOAD32_BYTE( "525-1105-06_4.1.1.bin", 0x000002, 0x010000, CRC(0dc3564f) SHA1(c86e640be0ef14636a4de065ab73b5671501c555) )
	ROM_LOAD32_BYTE( "525-1106-06_4.1.1.bin", 0x000003, 0x010000, CRC(4464a98b) SHA1(41fd033296904476b53dfe7513eb8da403d7acd4) )
ROM_END

// SPARCstation IPC (Sun 4/40)
/* SCC init 1 for the keyboard is identical to Sun 4/75 init 3 */
ROM_START( sun4_40 )
	ROM_REGION32_BE( 0x80000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "4.40_v2.9.rom", 0x0000, 0x40000, CRC(532fc20d) SHA1(d86d9e958017b3fecdf510d728a3e46a0ce3281d))
ROM_END

// SPARCstation IPX (Sun 4/50)
/* SCC init 1-2 for the keyboard is identical to Sun 4/75 init 1-2 */
ROM_START( sun4_50 )
	ROM_REGION32_BE( 0x80000, "user1", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v29", "V2.9" )
	ROMX_LOAD( "ipx-29.h1.u0501", 0x0000, 0x40000, CRC(1910aa65) SHA1(7d8832fea8e299b89e6ec7137fcde497673c14f8), ROM_BIOS(0)) // 525-1177-06(?) Boot (Version 2.9 version 20, supposedly?)
	ROM_SYSTEM_BIOS( 1, "v26", "V2.6" )
	ROMX_LOAD( "525-1177-05__=c=_sun_1992.am27c020.h1.u0501", 0x0000, 0x40000, CRC(aad28dee) SHA1(18075afa479fdc8d318df9aef9847dfb20591d79), ROM_BIOS(1)) // 525-1177-05 Boot (Version 2.6 version 410, supposedly?)
	ROM_SYSTEM_BIOS( 2, "v23", "V2.3" )
	ROMX_LOAD( "525-1177-03.h1.u0501", 0x0000, 0x40000, CRC(dcc1e66c) SHA1(a4dc3d8631aaa8416e22de273707c4ed7a2fe561), ROM_BIOS(2)) // 525-1177-03 Boot (Version 2.3)
ROM_END

// SPARCstation SLC (Sun 4/20)
/* SCC init 1 for the keyboard
 * :scc1 A Reg 09 <- 02 Master Interrupt Control - No Reset, No vector
 * :scc1 A Reg 04 <- 46 Setting up asynchronous frame format and clock, Parity Enable=0, Even Parity, Stop Bits 1, Clock Mode 16X
 * :scc1 A Reg 03 <- c0 Setting up the receiver, Receiver Enable 0, Auto Enables 0, Receiver Bits/Character 8
 * :scc1 A Reg 05 <- e2 Setting up the transmitter, Transmitter Enable 0, Transmitter Bits/Character 8, Send Break 0, RTS=1 DTR=1
 * :scc1 A Reg 0e <- 82 Misc Control Bits Baudrate Generator Input DPLL Command - not implemented
 * :scc1 A Reg 0b <- 55 Clock Mode Control 55 Clock type TTL level on RTxC pin, RCV CLK=BRG, TRA CLK=BRG, TRxC pin is Output, TRxC CLK=TRA CLK - not_implemented
 * :scc1 A Reg 0c <- 0e Low byte of Time Constant for Baudrate generator  -> 9600 baud
 * :scc1 A Reg 0d <- 00 High byte of Time Constant for Baudrate generator
 * :scc1 A Reg 03 <- c1 Setting up the receiver, Receiver Enable 1, Auto Enables 0, Receiver Bits/Character 8
 * :scc1 A Reg 05 <- ea Setting up the transmitter, Transmitter Enable 1, Transmitter Bits/Character 8, Send Break 0, RTS=1, DTR=1
 * :scc1 A Reg 0e <- 83 Misc Control Bits DPLL SRC=BRG Command - not implemented, BRG enabled SRC=PCLK, BRG SRC bps=38400=PCLK 4915200/128, BRG OUT 1200=38400/16
 * :scc1 A Reg 00 <- 10 Reset External/Status Interrupt
 * :scc1 A Reg 00 <- 01 Null command, register resetted by read of WR0
 * :scc1 A Reg 0c <- 0e Low byte of Time Constant for Baudrate generator  -> 9600 baud
 * :scc1 A Reg 00 <- 01 Null command, register resetted by read of WR0
 * :scc1 A Reg 0f <- c0 External/Status Control Bits, DCD Interrupt=1, Status FIFO enable=1, Zero detect interrupt:1 WR7 Prime enable:1 - not implemented
*/
ROM_START( sun4_20 )
	ROM_REGION32_BE( 0x80000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "520-2748-04.rom", 0x0000, 0x20000, CRC(e85b3fd8) SHA1(4cbc088f589375e2d5983f481f7d4261a408702e))
ROM_END

// SPARCstation 1 (Sun 4/60)
/* SCC init 1 for the keyboard is identical to Sun 4/75 init 3 */
ROM_START( sun4_60 )
	ROM_REGION32_BE( 0x80000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "ss1v29.rom", 0x0000, 0x20000, CRC(e3f103a9) SHA1(5e95835f1090ea94859bd005757f0e7b5e86181b))
ROM_END

// SPARCstation 1+ (Sun 4/65)
ROM_START( sun4_65 )
	ROM_REGION32_BE( 0x80000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "525-1108-05_1.3_ver_4.bin", 0x000000, 0x020000, CRC(67f1b3e2) SHA1(276ec5ca1dcbdfa202120560f55d52036720f87d) )
ROM_END

// SPARCstation 2 (Sun 4/75)
/* SCC init 1 for the keyboard
 *----------------------------
 * :scc1 A Reg 09 <- c0 Master Interrupt Control - Device reset  c0 A&B: RTS=1 DTR=1 INT=0
 * :scc1 int: 0
 * :scc1 A Reg 04 <- 46 Setting up asynchronous frame format and clock, Parity Enable=0, Even Parity, Stop Bits 1, Clock Mode 16X                                     * :scc1 A Reg 03 <- c0 Setting up the receiver, Receiver Enable 0, Auto Enables 0, Receiver Bits/Character 8
 * :scc1 A Reg 05 <- e2 Setting up the transmitter, Transmitter Enable 0, Transmitter Bits/Character 8, Send Break 0, RTS=1 DTR=1
 * :scc1 A Reg 09 <- 02 Master Interrupt Control - No reset  02 A&B: RTS=1 DTR=1 INT=0
 * :scc1 A Reg 0b <- 55 Clock Mode Control 55 Clock type TTL level on RTxC pin, RCV CLK=BRG, TRA CLK=BRG, TRxC pin is Output, TRxC CLK=TRA CLK - not_implemented
 * :scc1 A Reg 0c <- 7e Low byte of Time Constant for Baudrate generator
 * :scc1 A Reg 0d <- 00 High byte of Time Constant for Baudrate generator
 * :scc1 A Reg 0e <- 82 Misc Control Bits Baudrate Generator Input DPLL Command - not implemented
 * :scc1 A Reg 03 <- c1 Setting up the receiver, Receiver Enable 1, Auto Enables 0, Receiver Bits/Character 8
 * :scc1 A Reg 05 <- ea Setting up the transmitter, Transmitter Enable 1, Transmitter Bits/Character 8, Send Break 0, RTS=1, DTR=1
 * :scc1 A Reg 0e <- 83 Misc Control Bits DPLL SRC=BRG Command - not implemented, BRG enabled SRC=PCLK, BRG SRC bps=38400=PCLK 4915200/128, BRG OUT 1200=38400/16
 * :scc1 A Reg 00 <- 10 Reset External/Status Interrupt
 * :scc1 A Reg 00 <- 10 Reset External/Status Interrupt
 *
 * SCC init 2 for the keyboard - is Identical to init 1
 *
 * SCC init 3 for the keyboard - tricky one that reprogramms the baudrate constant as the last step.
 * -------------------------------------------------------------------------------------------------
 * :scc1 A Reg 09 <- 02 Master Interrupt Control - No Reset, No vector
 * :scc1 A Reg 04 <- 44 Setting up asynchronous frame format and clock, Parity Enable=0, Even Odd, Stop Bits 1, Clock Mode 16X
 * :scc1 A Reg 03 <- c0 Setting up the receiver, Receiver Enable 0, Auto Enables 0, Receiver Bits/Character 8
 * :scc1 A Reg 05 <- 60 Setting up the transmitter, Transmitter Enable 0, Transmitter Bits/Character 8, Send Break 0, RTS=0 DTR=0
 * :scc1 A Reg 0e <- 82 Misc Control Bits Baudrate Generator Input DPLL Command - not implemented
 * :scc1 A Reg 0b <- 55 Clock Mode Control 55 Clock type TTL level on RTxC pin, RCV CLK=BRG, TRA CLK=BRG, TRxC pin is Output, TRxC CLK=TRA CLK - not_implemented
 * :scc1 A Reg 0c <- 0e Low byte of Time Constant for Baudrate generator  -> 9600 baud
 * :scc1 A Reg 0d <- 00 High byte of Time Constant for Baudrate generator
 * :scc1 A Reg 03 <- c1 Setting up the receiver, Receiver Enable 1, Auto Enables 0, Receiver Bits/Character 8
 * :scc1 A Reg 05 <- 68 Setting up the transmitter, Transmitter Enable 1, Transmitter Bits/Character 8, Send Break 0, RTS=0, DTR=0
 * :scc1 A Reg 0e <- 83 Misc Control Bits DPLL SRC=BRG Command - not implemented, BRG enabled SRC=PCLK, BRG SRC bps=307200=PCLK 4915200/16, BRG OUT 9600=307200/16
 * :scc1 A Reg 00 <- 10 Reset External/Status Interrupt
 * :scc1 A Reg 00 <- 10 Reset External/Status Interrupt
 * :scc1 A Reg 0c <- 7e Low byte of Time Constant for Baudrate generator -> 1200 baud
*/
ROM_START( sun4_75 )
	ROM_REGION32_BE( 0x80000, "user1", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v29", "V2.9" )
	ROMX_LOAD( "ss2-29.rom", 0x0000, 0x40000, CRC(d04132b3) SHA1(ef26afafa2800b8e2e5e994b3a76ca17ce1314b1), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v22", "V2.2" )
	ROMX_LOAD( "525-1107-06.rom", 0x0000, 0x40000, CRC(7f5b58b4) SHA1(10a3eb3ddee667e7cf3c04aef6f6549e1b7f8311), ROM_BIOS(1) )
ROM_END

// SPARCstation 10 (Sun S10)
ROM_START( sun_s10 )
	ROM_REGION32_BE( 0x80000, "user1", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "r225", "Rev 2.2.5")
	ROMX_LOAD( "ss10_v2.25.rom", 0x0000, 0x80000, CRC(c7a48fd3) SHA1(db13d85b02f181eb7fce4c38b11996ff64116619), ROM_BIOS(0))
	// SPARCstation 10 and 20
	ROM_SYSTEM_BIOS(1, "r225r", "Rev 2.2.5r")
	ROMX_LOAD( "ss10-20_v2.25r.rom", 0x0000, 0x80000, CRC(105ba132) SHA1(58530e88369d1d26ab11475c7884205f2299d255), ROM_BIOS(1))
ROM_END

// SPARCstation 20
ROM_START( sun_s20 )
	ROM_REGION32_BE( 0x80000, "user1", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "r225", "Rev 2.2.5")
	ROMX_LOAD( "ss20_v2.25.rom", 0x0000, 0x80000, CRC(b4f5c547) SHA1(ee78312069522094950884d5bcb21f691eb6f31e), ROM_BIOS(0))
	// SPARCstation 10 and 20
	ROM_SYSTEM_BIOS(1, "r225r", "Rev 2.2.5r")
	ROMX_LOAD( "ss10-20_v2.25r.rom", 0x0000, 0x80000, CRC(105ba132) SHA1(58530e88369d1d26ab11475c7884205f2299d255), ROM_BIOS(1))
ROM_END

void sun4_state::init_sun4()
{
	m_arch = ARCH_SUN4;
}

void sun4_state::init_sun4c()
{
	m_arch = ARCH_SUN4C;
}

/* Drivers */

//    YEAR  NAME      PARENT    COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY             FULLNAME                       FLAGS
// sun4
COMP( 198?, sun4_110, 0,        0,      sun4,    sun4,  sun4_state, init_sun4,  "Sun Microsystems", "Sun 4/110",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1987, sun4_300, 0,        0,      sun4,    sun4,  sun4_state, init_sun4,  "Sun Microsystems", "Sun 4/3x0",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 198?, sun4_400, 0,        0,      sun4,    sun4,  sun4_state, init_sun4,  "Sun Microsystems", "Sun 4/4x0",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

// sun4c
COMP( 1990, sun4_40,  sun4_300, 0,      sun4_40, sun4,  sun4_state, init_sun4c, "Sun Microsystems", "SPARCstation IPC (Sun 4/40)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
COMP( 1991, sun4_50,  sun4_300, 0,      sun4_50, sun4,  sun4_state, init_sun4c, "Sun Microsystems", "SPARCstation IPX (Sun 4/50)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 199?, sun4_20,  sun4_300, 0,      sun4_20, sun4,  sun4_state, init_sun4c, "Sun Microsystems", "SPARCstation SLC (Sun 4/20)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1989, sun4_60,  sun4_300, 0,      sun4_60, sun4,  sun4_state, init_sun4c, "Sun Microsystems", "SPARCstation 1 (Sun 4/60)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
COMP( 1990, sun4_65,  sun4_300, 0,      sun4_65, sun4,  sun4_state, init_sun4c, "Sun Microsystems", "SPARCstation 1+ (Sun 4/65)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1990, sun4_75,  sun4_300, 0,      sun4_75, sun4,  sun4_state, init_sun4c, "Sun Microsystems", "SPARCstation 2 (Sun 4/75)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

// sun4m (using the SPARC "reference MMU", probably will go to a separate driver)
COMP( 1992, sun_s10,  sun4_300, 0,      sun4c,   sun4,  sun4_state, init_sun4c, "Sun Microsystems", "SPARCstation 10 (Sun S10)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1994, sun_s20,  sun4_300, 0,      sun4c,   sun4,  sun4_state, init_sun4c, "Sun Microsystems", "SPARCstation 20",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

