// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Force SYS68K CPU-30 VME SBC drivers
 *
 *  21/05/2016
 *
 * Thanks to Al Kossow his site http://www.bitsavers.org/ I got the information
 * required to start the work with this driver.
 *
 * The driver is currently starting up and the Boot ROM asks for input do start FGA-002
 * diagnostics and do SRAM setup, which it does. After that it jumps to zeroed memory
 * and crashes so needs some more work to be useful
 *
 *
 *       ||
 * ||    ||  CPU-30
 * ||||--||_____________________________________________________________
 * ||||--||                                                             |
 * ||    ||                                                           _ |__
 *       ||                                +----------------+        | |   |
 * RST O===                                |                |        | |   |
 *       ||                                |                |        | |   |
 * ABT O===                                |    FGA-002     |        | |   |
 RUN/HLT C| GREEN/RED    +------------+    |                |        | |   |
 BUS MST C| GREEN/unlit  |System flash|    |                |        | |   |
 *       ||              +------------+    |                |        | |VME|
 * rot   [O SW4          +------------+    +----------------+        | |   |
 * swtshs||              |System Flash|                              | |P1 |
 *       [O SW3          +------------+            +----------+      | |   |
 *       ||              +------------+            |          |      | |   |
 *       ||              |System Flash|            | 68030    |      | |   |
 *     | ||              +------------+            |          |      | |   |
 *     |--|              +------------+            |          |      | |   |
 * AUI |  |              |System Flash|            +----------+      | |   |
 *  eth|  |              +------------+                              |_|   |
 *     |  |                                        +----------+        |___|
 *     |  |      +-----+                           |          |        |
 *     |--|      |SIA  |                           | 68882    |        |
 *     | ||      |7992 |                           |          |        |
 *       ||      +-----+         J28               |          |        |
 *       ||                  +------+              +----------+        |
 *       ||                  | Opt  |                                  |
 *      |||                  | Boot |                                  |
 *     +---+                 | PROM |                                  |
 * Ser |   =   +----------+  +------+              +--------+          |___
 * #3  |   =   | LANCE    |  +------+              | SCSI   |       _ _|   |
 *     |   =   | 7970     |  | Def  |              | 87034  |      |   |   |
 *     +---+   |          |  | Boot |              |        |      |   |   |
 *      |||    |          |  | PROM |              +--------+      |   |   |
 *       ||    +----------+  +------+                              |   |   |
 *      |||    +----+            J36                 +------+      |   |VME|
 *     +---+   |NV- |                                | FDC  |      |   |   |
 * Ser |   =   | RAM|                                |37C65C|      |   |P2 |
 * #2  |   =   |    |                                +------+      |   |   |
 *     |   =   +----+                                              | 5 row |
 *     +---+                             +--------+  +--------+    | conn- |
 *      |||     +-----+                  | DUSCC1 |  | PIT1   |    |  ector|
 *       ||     | OPT |                  | 68562  |  | 68230  |    |   |   |
 *      |||     |NVRAM|                  |        |  |        |    |   |   |
 *     +---+    |     |                  +--------+  +--------+    |   |   |
 * Ser |   =    |     |   ____           +--------+  +--------+    |   |   |
 * #1  |   =    |     |  /    \          | DUSCC2 |  | PIT2   |    |_ _|   |
 *     |   =    |     |  BATTERY         | 68562  |  | 68230  |        |___|
 * ||  +---+    +-----+  \____/          |        |  |        |          +
 * ||||--||                              +--------+  +--------+          |
 * ||||--||--------------------------------------------------------------+
 * ||
 *
 * History of Force Computers
 *---------------------------
 *
 * Misc links about Force Computes and this board:
 *------------------------------------------------
 * http://bitsavers.trailing-edge.com/pdf/forceComputers/CPU30/204030_CPU-30_R4_Technical_Reference_Oct96.pdf
 * http://www.artisantg.com/info/P_wUovN.pdf
 *
 * Description(s)
 * -------------
 * CPU-30 has the following feature set
 *      -  16.7 or 25 MHz MC68030 enhanced 32-bit microprocessor
 *      -  16.7 or 25 MHz MC68882 floating-point coprocessor
 *      -  32-512 Kb of SRAM with battery backup
 *      -  4, 8, 16, or 32MB of shared DRAM, with byte parity
 *      -  Up to 8Mb Flash memory
 *      -  128,256 or 512 Mb boot flash or upto 1Mb of boot OTP PROM
 *      -  Double High (6U) VMEmodule
 *      -  A32/D32 VMEbus master/slave interface with system controller function (VMEchip ASIC)
 *      -  Ethernet transceiver interface (AM79C90)
 *      -  SCSI bus interface with independent data bus on P2 connector (MB87033/34)
 *      -  Flopyy disk interface on P2 connector (FCD37C65C)
 *      -  Four serial ports (DUSCC SCN68562 x 2)
 *      -  20 bit digital i/o for user applications( 2 x 68230 PI/T )
 *      -  Real-Time Clock with interrupt (72423)
 *      -  4-level requester, 7-level interrupter, and 7-level interrupt handler for VMEbus (VMEchip ASIC)
 *
 * NOTE: This driver currently mimics the CPU-30xyz configuration: 16MHz, 4Mb RAM, no parity, no ethernet (See TODO)
 *
 * Address Map
 * --------------------------------------------------------------------------
 *  Range                   Decscription
 * --------------------------------------------------------------------------
 * 00000000-0xxFFFFF        Shared DRAM D8-D32 xx=0x1F-0x03 for 32Mb-4Mb
 * 0yy00000-FAFFFFFF        VME A32 D8-D32     yy=xx+1
 * FB000000-FBFEFFFF        VME A24 D8-D32
 * FBFF0000-FBFFFFFF        VME A16 D8-D32
 * FC000000-FCFEFFFF        VME A24 D8-D16
 * FCFF0000-FCFFFFFF        VME A16 D8-D16
 * FD000000-FEEFFFFF        Reserved
 * FEF00000-FEF7FFFF        LAN RAM D8-D32
 * FEF80000-FEFFFFFF        LAN Controller D16 (AM79C90)
 * FF000000-FF7FFFFF        System PROM D8-D32 (read) D32 (flash write)
 * FF800000-FF800BFF        Reserved
 * FF800C00-FF800DFF        PIT1 D8 (68230)
 * FF800E00-FF800FFF        PIT2 D8 (68230)
 * FF801000-FF801FFF        Reserved
 * FF802000-FF8021FF        DUSCC1 D8 (SCN68562)
 * FF802200-FF8023FF        DUSCC2 D8 (SCN68562)
 * FF802400-FF802FFF        Reserved
 * FF803000-FF8031FF        RTC (72423) D8
 * FF803200-FF8033FF        Reserved
 * FF803400-FF8035FF        SCSI controller (MB87033/34) D8
 * FF803600-FF8037FF        Reserved
 * FF803800-FF80397F        Floppy controller (FDC37C65C) D8
 * FF803980-FF8039FF        Slot 1 status register (read) D8
 * FFC00000-FFCFFFFF        Local SRAM D8-D32
 * FFD00000-FFDFFFFF        FGA-002 Gate Array D8-D32
 * FFE00000-FFEFFFFF        Boot PROM D8-D32
 * FFF00000-FFFFFFFF        Reserved
 * --------------------------------------------------------------------------
 *
 * Interrupt sources CPU30
 * ----------------------------------------------------------
 * Description                  Device  Lvl  IRQ    VME board
 *                           /Board      Vector  Address
 * ----------------------------------------------------------
 * On board Sources
 *
 * Off board Sources (other VME boards)
 *
 * ----------------------------------------------------------
 *
 * PIT #1 hardware wiring
 * ----------------------------------------------------------
 * PA0-PA3  Rotary Switch 1 input
 * PA4-PA7  Rotary Switch 2 input
 * H1-H4    User I/O via optional B5 or optional 5-row VME P2 connector
 * PB0-PB2  Floppy Disk Drive Control output
 * PB3-PB4  Floppy Disk Drive Control input
 * PB5      Floppy Disk Drive Control output
 * PB6-PB7  DMAC control
 * PC0,PC1  User I/O via optional B5 or optional 5-row VME P2 connector
 * PC4,PC7  User I/O via optional B5 or optional 5-row VME P2 connector
 * PC2      Reserved
 * PC3      Timer interrupt request
 * PC5      Port Interrupt Request
 * PC6      68882 FPCP Sense
 *
 * PIT #2 hardware setup wiring
 * ----------------------------------------------------------
 * PA0-PA7  User I/O via optional B5 or optional 5-row VME P2 connector
 * H1-H4    User I/O via optional B5 or optional 5-row VME P2 connector
 * PB0-PB2  Memory Size
 * PB3-PB7  Board ID
 * PC0-PC1  Hardware ID
 * PC2      Status of write protection for (default and optional) Boot PROMs
 * PC3      Timer Interrupt request
 * PC4      Status of write protection for SYSTEM-Flash Memory
 * PC5      DMA control
 * PC6      Flash programming voltage control
 * PC7      Reserved
 *
 * DMAC Channel Assignments
 * ----------------------------------------------------------
 * Channel         SYS68K/CPU30
 * ----------------------------------------------------------
 *
 *
 *  TODO:
 *  - Investigate and fix crash
 *  - Add VxWorks proms
 *  - Add more devices
 *  - Write VME device
 *  - Add variants of boards
 *
 ****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/scnxx562.h"
#include "machine/68230pit.h"
#include "machine/fga002.h"
#include "machine/nvram.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
//#include "machine/timekpr.h"

#define VERBOSE 0

#define LOGPRINT(x)  do { if (VERBOSE) logerror x; } while (0)
#define LOG(x)
#define LOGR(x)
#define LOGSETUP(x)
#define LOGINT(x) LOGPRINT(x)
#if VERBOSE >= 2
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define DUSCC_CLOCK XTAL_14_7456MHz /* Verified */

class fccpu30_state : public driver_device
{
public:
fccpu30_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device (mconfig, type, tag)
		, m_maincpu (*this, "maincpu")
		, m_dusccterm(*this, "duscc")
		, m_pit1 (*this, "pit1")
		, m_pit2 (*this, "pit2")
		, m_fga002 (*this, "fga002")
	{
	}

	uint32_t bootvect_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void bootvect_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	/* Interrupt  support */
	//  int maincpu_iack_callback(device_t &device, int irqline);
	void fga_irq_callback(int state);
	uint8_t fga_irq_state;
	//  int fga_irq_vector;
	int fga_irq_level;

	/* Rotary switch PIT input */
	uint8_t rotary_rd(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t flop_dmac_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void flop_dmac_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pit1c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pit1c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pit2a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pit2a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t board_mem_id_rd(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pit2c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pit2c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	/* VME bus accesses */
	//uint16_t vme_a24_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	//void vme_a24_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	//uint16_t vme_a16_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	//void vme_a16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	virtual void machine_start () override;
	virtual void machine_reset () override;
protected:

private:
	required_device<cpu_device> m_maincpu;
	required_device<duscc68562_device> m_dusccterm;

	required_device<pit68230_device> m_pit1;
	required_device<pit68230_device> m_pit2;

	required_device<fga002_device> m_fga002;

	// Helper functions
	void update_irq_to_maincpu();

	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	uint32_t  *m_sysrom;
	uint32_t  m_sysram[2];
};

static ADDRESS_MAP_START (fccpu30_mem, AS_PROGRAM, 32, fccpu30_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x00000000, 0x00000007) AM_ROM AM_READ  (bootvect_r)   /* ROM mirror just during reset */
	AM_RANGE (0x00000000, 0x00000007) AM_RAM AM_WRITE (bootvect_w)   /* After first write we act as RAM */
	AM_RANGE (0x00000008, 0x003fffff) AM_RAM /* 4 Mb RAM */
	AM_RANGE (0xff000000, 0xff7fffff) AM_ROM AM_REGION("roms", 0x000000)
	AM_RANGE (0xff802000, 0xff8021ff) AM_DEVREADWRITE8("duscc", duscc68562_device, read, write, 0xffffffff) /* Port 1&2 - Dual serial port DUSCC   */
	AM_RANGE (0xff802200, 0xff8023ff) AM_DEVREADWRITE8("duscc2", duscc68562_device, read, write, 0xffffffff) /* Port 3&4 - Dual serial port DUSCC   */
	AM_RANGE (0xff800c00, 0xff800dff) AM_DEVREADWRITE8("pit1", pit68230_device, read, write, 0xffffffff)
	AM_RANGE (0xff800e00, 0xff800fff) AM_DEVREADWRITE8("pit2", pit68230_device, read, write, 0xffffffff)
	AM_RANGE (0xffc00000, 0xffcfffff) AM_RAM AM_SHARE ("nvram") /* On-board SRAM with battery backup (nvram) */
	AM_RANGE (0xffd00000, 0xffdfffff) AM_DEVREADWRITE8("fga002", fga002_device, read, write, 0xffffffff)  /* FGA-002 Force Gate Array */
	AM_RANGE (0xffe00000, 0xffefffff) AM_ROM AM_REGION("roms", 0x800000)

	//AM_RANGE(0x100000, 0xfeffff)  AM_READWRITE(vme_a24_r, vme_a24_w) /* VMEbus Rev B addresses (24 bits) - not verified */
	//AM_RANGE(0xff0000, 0xffffff)  AM_READWRITE(vme_a16_r, vme_a16_w) /* VMEbus Rev B addresses (16 bits) - not verified */
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START (fccpu30)
INPUT_PORTS_END

/* Start it up */
void fccpu30_state::machine_start ()
{
	LOG(("--->%s\n", FUNCNAME));

	save_pointer (NAME (m_sysrom), sizeof(m_sysrom));
	save_pointer (NAME (m_sysram), sizeof(m_sysram));

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (uint32_t*)(memregion ("roms")->base () + 0x800000);
}

void fccpu30_state::machine_reset ()
{
	LOG(("--->%s\n", FUNCNAME));

	/* Reset pointer to bootvector in ROM for bootvector handler bootvect_r */
	if (m_sysrom == &m_sysram[0]) /* Condition needed because memory map is not setup first time */
		m_sysrom = (uint32_t*)(memregion ("roms")->base () + 0x800000);
}

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0xff800000 to 0x0 at reset*/
uint32_t fccpu30_state::bootvect_r(address_space &space, offs_t offset, uint32_t mem_mask){
	return m_sysrom[offset];
}

void fccpu30_state::bootvect_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask){
	m_sysram[offset % sizeof(m_sysram)] &= ~mem_mask;
	m_sysram[offset % sizeof(m_sysram)] |= (data & mem_mask);
	m_sysrom = &m_sysram[0]; // redirect all upcomming accesses to masking RAM until reset.
}

/*
 * Rotary Switches - to configure the board
 *
 * Table 25: PI/T #1 Interface Signals
 * Pin     Function  In/Out
 * PA0-PA3   SW1      In
 * PA4 PA7   SW2      In
 *
 * Table 38: Upper Rotary Switch (SW2)
 * Bit 3: This bit indicates whether the RAM disk should be initialized after reset. If this bit is set to "0" (settings 0-7),
 *  the RAM disk is initialized as defined by bit 0 and 1. When the disk is initialized, all data on the disk is lost.
 * Bit 2: This bit defines the default data size on the VMEbus. If the bit is set to "0", 16 bits are selected, if it is set
 *  to "1", 32 bits are selected.
 * Bit 1 and Bit 0: These two bits define the default RAM disk. See Table 40, "RAM Disk Usage," a detailed description.
 *  If AUTOBOOT is set by bit 2 and 3 of SW1, bit 1 and 0 of SW2 define which operating system will be booted. See Table 42,
 *  "Boot an Operating System (if AUTOBOOT is selected)," on page 129 for detailed description.
 *
 * Table 39: Lower Rotary Switch (SW1)
 * Bit 3 and Bit 2: These two bits define which program is to be invoked after reset. Please refer
 *  to Table 41, "Program After Reset," on page 129 for a detailed description.
 * Bit 1: If this switch is "0" (settings 0,1,4,5,8,9,C,D), VMEPROM tries to execute a start-up file after reset. The default
 *  filename is SY$STRT. If the bit is "1", VMEPROM comes up with the default banner.
 * Bit 0: If this switch is set to "0" (settings 0,2,4,6,8,A,C,E), VMEPROM checks the VMEbus for available hardware after reset.
 *  In addition VMEPROM waits for SYSFAIL to disappear from the VMEbus. The following hardware can be detected:
 *  - Contiguous memory
 *  - ASCU-1/2
 *  - ISIO-1/2
 *  - SIO-1/2
 *  - ISCSI-1
 *  - WFC-1
 *
 * Table 40: RAM Disk Usage
 * Bit 1 Bit 0 Upper Switch (SW 2) selected on
 *  1     1     RAM DISK AT TOP OF MEMORY (32 Kbytes) 3,7,B,F
 *  1     0     RAM DISK AT 0xFC80 0000 (512 Kbytes) 2,6,A,E
 *  0     1     RAM DISK AT 0x4070 0000 (512 Kbytes) 1,5,9,D
 *  0     0     RAM DISK AT 0x4080 0000 (512 Kbytes) 0,4,8,C
 *
 * Table 41: Program After Reset
 * Bit 3 Bit 2 Lower Switch (SW 1)          selected on
 *  1     1     VMEPROM                         C,D,E,F
 *  1     0     USER PROGRAM AT 0x4070 0000     8,9,A,B
 *  0     1     AUTOBOOT SYSTEM                 4,5,6,7
 *  0     0     USER PROGRAM AT 4080.000016     0,1,2,3
 *
 * Table 42: Boot an Operating System (if AUTOBOOT is selected)
 * Bit 1 Bit 0 Upper Switch (SW 2)          selected on
 *  1     1     reserved                        3,7,B,F
 *  1     0     Boot UNIX/PDOS 4.x              2,6,A,E
 *  0     1     Boot another operating system   1,5,9,D
 *  0     0     Setup for UNIX mailbox driver   0,4,8,C
 *
 * "To start VMEPROM, the rotary switches must both be set to 'F':" Hmm...
 */
uint8_t fccpu30_state::rotary_rd(address_space &space, offs_t offset, uint8_t mem_mask){
	LOG(("%s\n", FUNCNAME));
	return 0xff; // TODO: make this configurable from commandline or artwork
}

// PIT#1 Port B TODO: implement floppy and dma control
uint8_t fccpu30_state::flop_dmac_r(address_space &space, offs_t offset, uint8_t mem_mask){
	LOG(("%s\n", FUNCNAME));
	return 0xff;
}

void fccpu30_state::flop_dmac_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask){
	LOG(("%s(%02x)\n", FUNCNAME, data));
}

// PIT#1 Port C TODO: implement timer+port interrupts and 68882 sense
// TODO: Connect PC0, PC1, PC4 and PC7 to B5 and/or P2 connector
uint8_t fccpu30_state::pit1c_r(address_space &space, offs_t offset, uint8_t mem_mask){
	LOG(("%s\n", FUNCNAME));
	return 0xff;
}

void fccpu30_state::pit1c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask){
	LOG(("%s(%02x)\n", FUNCNAME, data));
}

// PIT#2 Port A TODO: Connect to B5 and /or P2 connector
uint8_t fccpu30_state::pit2a_r(address_space &space, offs_t offset, uint8_t mem_mask){
	LOG(("%s\n", FUNCNAME));
	logerror("Unsupported user i/o on PIT2 port A detected\n");
	return 0xff;
}

void fccpu30_state::pit2a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask){
	LOG(("%s(%02x)\n", FUNCNAME, data));
	logerror("Unsupported user i/o on PIT2 port A detected\n");
}

/*
 * PI/T #2 Factory settings
 * B0-B2 Shared Memory Size - From these lines, the on-board Shared RAM capacity can be read in by software.
 * 0 0 0 32 Mb
 * 0 0 1 16 Mb
 * 0 1 0  8 Mb
 * 0 1 1  4 Mb
 * 1 x x  Reserved
 * B3-B7 Board ID(s) -  From these lines, the CPU board identification number can be read in by
 * 0 1 0 1 0 CPU-30 R4  software. Every CPU board has a unique number. Different versions of
 * (fill in more)       one CPU board (i.e. different speeds, capacity of memory, or modules)
 *                      contain the same identification number. In the case of the CPU-30 R4, the
 *                      number is ten ("10" decimal or 0A16 hexadecimal "01010" binary).
 */
uint8_t fccpu30_state::board_mem_id_rd(address_space &space, offs_t offset, uint8_t mem_mask){
	LOG(("%s\n", FUNCNAME));
	//  return 0x6A; // CPU-30 R4 with 4Mb of shared RAM. TODO: make this configurable from commandline or artwork
	//  return 0x57; // blankt 53 56
	//      return 0x36; // CPU-26 1Mb 36 MHz
	//      return 0x35; // CPU-26 1Mb 36 MHz
			return 0x36; // CPU-26 1Mb 36 MHz
}

// PIT#2 Port C TODO: implement timer interrupt, DMA i/o, memory control and Hardware ID
uint8_t fccpu30_state::pit2c_r(address_space &space, offs_t offset, uint8_t mem_mask){
	LOG(("%s\n", FUNCNAME));
	return 0xff;
}

void fccpu30_state::pit2c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask){
	LOG(("%s(%02x)\n", FUNCNAME, data));
}

#if 0
/* Dummy VME access methods until the VME bus device is ready for use */
uint16_t fccpu30_state::vme_a24_r(address_space &space, offs_t offset, uint16_t mem_mask){
	LOG (logerror ("vme_a24_r\n"));
	return (uint16_t) 0;
}

void fccpu30_state::vme_a24_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask){
	LOG (logerror ("vme_a24_w\n"));
}

uint16_t fccpu30_state::vme_a16_r(address_space &space, offs_t offset, uint16_t mem_mask){
	LOG (logerror ("vme_16_r\n"));
	return (uint16_t) 0;
}

void fccpu30_state::vme_a16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask){
	LOG (logerror ("vme_a16_w\n"));
}
#endif

void fccpu30_state::fga_irq_callback(int state)
{
	LOGINT(("%s(%02x)\n", FUNCNAME, state));

	fga_irq_state = state;
	fga_irq_level = m_fga002->get_irq_level();
	LOGINT((" - FGA irq level  %02x\n", fga_irq_level));
	update_irq_to_maincpu();
}

void fccpu30_state::update_irq_to_maincpu()
{
	LOGINT(("%s()\n", FUNCNAME));
	LOGINT((" - fga_irq_level: %02x\n", fga_irq_level));
	LOGINT((" - fga_irq_state: %02x\n", fga_irq_state));
	switch (fga_irq_level & 0x07)
	{
	case 1: m_maincpu->set_input_line(M68K_IRQ_1, fga_irq_state); break;
	case 2: m_maincpu->set_input_line(M68K_IRQ_2, fga_irq_state); break;
	case 3: m_maincpu->set_input_line(M68K_IRQ_3, fga_irq_state); break;
	case 4: m_maincpu->set_input_line(M68K_IRQ_4, fga_irq_state); break;
	case 5: m_maincpu->set_input_line(M68K_IRQ_5, fga_irq_state); break;
	case 6: m_maincpu->set_input_line(M68K_IRQ_6, fga_irq_state); break;
	case 7: m_maincpu->set_input_line(M68K_IRQ_7, fga_irq_state); break;
	default: logerror("Programmatic error in %s, please report\n", FUNCNAME);
	}
}

/*
 * Machine configuration
 */
static MACHINE_CONFIG_START (fccpu30, fccpu30_state)
	/* basic machine hardware */
	MCFG_CPU_ADD ("maincpu", M68030, XTAL_25MHz)
	MCFG_CPU_PROGRAM_MAP (fccpu30_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("fga002", fga002_device, iack)
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Terminal Port config */
	/* Force CPU30 series of boards has up to four serial ports, p1-p4, the FGA boot uses p4 as console and subsequent
	   firmware uses p1 as console and in an operating system environment there may be user login shells on the other.

	   In order to use more than just one terminal MAME supports serial socket servers to which it is possible to
	   connect a telnet terminal to. The general syntax to open a socket server from the command prompts is:

	   mame  fccpu30 -window -rs232p4 null_modem -bitbngr socket.127.0.0.1:1001

	   At the opening screen, before the board starts to execute code, start up the telnet client and give 127.0.0.1:1001 as host
	   It is also possible to enumerate more than one terminal server in order to have several terminal session attached.

	   mame  fccpu30 -window -rs232p4 null_modem -bitbngr1 socket.127.0.0.1:1001 -rs232p1 null_modem -bitbngr2 socket.127.0.0.1:1002

	   Now just start up the telnet clients with 127.0.0.1:1001 and 127.0.0.1:1002 as hosts and you have control of input for each port.
	*/

#define RS232P1_TAG      "rs232p1"
#define RS232P2_TAG      "rs232p2"
#define RS232P3_TAG      "rs232p3"
#define RS232P4_TAG      "rs232p4"

	MCFG_DUSCC68562_ADD("duscc", DUSCC_CLOCK, 0, 0, 0, 0 )
	/* Port 1 on Port B */
	MCFG_DUSCC_OUT_TXDB_CB(DEVWRITELINE(RS232P1_TAG, rs232_port_device, write_txd))
	MCFG_DUSCC_OUT_DTRB_CB(DEVWRITELINE(RS232P1_TAG, rs232_port_device, write_dtr))
	MCFG_DUSCC_OUT_RTSB_CB(DEVWRITELINE(RS232P1_TAG, rs232_port_device, write_rts))
	/* Port 4 on Port A */
	MCFG_DUSCC_OUT_TXDA_CB(DEVWRITELINE(RS232P4_TAG, rs232_port_device, write_txd))
	MCFG_DUSCC_OUT_DTRA_CB(DEVWRITELINE(RS232P4_TAG, rs232_port_device, write_dtr))
	MCFG_DUSCC_OUT_RTSA_CB(DEVWRITELINE(RS232P4_TAG, rs232_port_device, write_rts))
	/* DUSCC1 interrupt signal REQN is connected to LOCAL IRQ4 of the FGA-002 and level is programmable */
	MCFG_DUSCC_OUT_INT_CB(DEVWRITELINE("fga002", fga002_device, lirq4_w))

	MCFG_DUSCC68562_ADD("duscc2", DUSCC_CLOCK, 0, 0, 0, 0 )
	/* Port 2 on Port A */
	MCFG_DUSCC_OUT_TXDA_CB(DEVWRITELINE(RS232P2_TAG, rs232_port_device, write_txd))
	MCFG_DUSCC_OUT_DTRA_CB(DEVWRITELINE(RS232P2_TAG, rs232_port_device, write_dtr))
	MCFG_DUSCC_OUT_RTSA_CB(DEVWRITELINE(RS232P2_TAG, rs232_port_device, write_rts))
	/* Port 3 on Port B */
	MCFG_DUSCC_OUT_TXDB_CB(DEVWRITELINE(RS232P3_TAG, rs232_port_device, write_txd))
	MCFG_DUSCC_OUT_DTRB_CB(DEVWRITELINE(RS232P3_TAG, rs232_port_device, write_dtr))
	MCFG_DUSCC_OUT_RTSB_CB(DEVWRITELINE(RS232P3_TAG, rs232_port_device, write_rts))
	/* DUSCC2 interrupt signal REQN is connected to LOCAL IRQ5 of the FGA-002 and level is programmable */
	MCFG_DUSCC_OUT_INT_CB(DEVWRITELINE("fga002", fga002_device, lirq5_w))

	MCFG_RS232_PORT_ADD (RS232P1_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("duscc", duscc68562_device, rxb_w))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("duscc", duscc68562_device, ctsb_w))

	MCFG_RS232_PORT_ADD (RS232P2_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("duscc2", duscc68562_device, rxa_w))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("duscc2", duscc68562_device, ctsa_w))

	MCFG_RS232_PORT_ADD (RS232P3_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("duscc2", duscc68562_device, rxb_w))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("duscc2", duscc68562_device, ctsb_w))

	MCFG_RS232_PORT_ADD (RS232P4_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("duscc", duscc68562_device, rxa_w))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("duscc", duscc68562_device, ctsa_w))

	/* PIT Parallel Interface and Timer device, assumed strapped for on board clock */
	MCFG_DEVICE_ADD ("pit1", PIT68230, XTAL_16MHz / 2) // The PIT clock is not verified on schema but reversed from behaviour
	MCFG_PIT68230_PA_INPUT_CB(READ8(fccpu30_state, rotary_rd))
	MCFG_PIT68230_PB_INPUT_CB(READ8(fccpu30_state, flop_dmac_r))
	MCFG_PIT68230_PB_OUTPUT_CB(WRITE8(fccpu30_state, flop_dmac_w))
	MCFG_PIT68230_PC_INPUT_CB(READ8(fccpu30_state, pit1c_r))
	MCFG_PIT68230_PC_OUTPUT_CB(WRITE8(fccpu30_state, pit1c_w))
// MCFG_PIT68230_OUT_INT_CB(DEVWRITELINE("fga002", fga002_device, lirq2_w)) // Interrupts not yet supported by 68230

	MCFG_DEVICE_ADD ("pit2", PIT68230, XTAL_16MHz / 2) // Th PIT clock is not verified on schema but reversed from behaviour
	MCFG_PIT68230_PB_INPUT_CB(READ8(fccpu30_state, board_mem_id_rd))
	MCFG_PIT68230_PA_INPUT_CB(READ8(fccpu30_state, pit2a_r))
	MCFG_PIT68230_PA_OUTPUT_CB(WRITE8(fccpu30_state, pit2a_w))
	MCFG_PIT68230_PC_INPUT_CB(READ8(fccpu30_state, pit2c_r))
	MCFG_PIT68230_PC_OUTPUT_CB(WRITE8(fccpu30_state, pit2c_w))
// MCFG_PIT68230_OUT_INT_CB(DEVWRITELINE("fga002", fga002_device, lirq3_w)) // Interrupts not yet supported by 68230

	/* FGA-002, Force Gate Array */
	MCFG_FGA002_ADD("fga002", 0)
	MCFG_FGA002_OUT_INT_CB(WRITELINE(fccpu30_state, fga_irq_callback))
	MCFG_FGA002_OUT_LIACK4_CB(DEVREAD8("duscc",  duscc_device, iack))
	MCFG_FGA002_OUT_LIACK5_CB(DEVREAD8("duscc2",  duscc_device, iack))
MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (fccpu30)
	ROM_REGION32_BE(0x900000, "roms", 0)

#if 1
// Boots with Board ID set to: 0x36  (FGA002 BOOT and VMEPROM on terminal P4)
	ROM_LOAD16_BYTE("CPU33LO.BIN",  0x000001, 0x40000, CRC (49895fdf) SHA1 (733abd144c95225a2faf920490e31df2a27f8e03))
	ROM_LOAD16_BYTE("CPU33UP.BIN",  0x000000, 0x40000, CRC (cfe75e94) SHA1 (d40e0635a48607be25f7c58c74b53b7e58fe735d))

// Boots with Board ID set to: 0x36 (FGA002 BOOT on terminal P4, "Wait until harddisk is up to speed " on terminal P1)
#else
	ROM_LOAD32_BYTE("CPU30LL.BIN",  0x000003, 0x20000, CRC (a03ebf46) SHA1 (48fa0268cb10e20679c093e02574dbd9925f95d1))
	ROM_LOAD32_BYTE("CPU30LO.BIN",  0x000002, 0x20000, CRC (fefa88ed) SHA1 (71a9ad807c0c2da5c6f6a6dc68c73ad8b52f3ea9))
	ROM_LOAD32_BYTE("CPU30UP.BIN",  0x000001, 0x20000, CRC (dfed1f68) SHA1 (71478a77d5ab5da0fabcd78e69537919b560e3b8))
	ROM_LOAD32_BYTE("CPU30UU.BIN",  0x000000, 0x20000, CRC (66e95cc2) SHA1 (acdb468a3a5974295b81271d617de7f101098891))
#endif

// Same binary for many boards, attempts to detect CPU speed etc, currently failing detection but boots system roms anyway
	ROM_LOAD       ("PGA-002.bin",  0x800000, 0x10000, CRC(faa38972) SHA1(651dfc2f9a865fc6adf49dad90f9e705f2889919) )
ROM_END

/*
 * System ROM information
 *
 * FGA-002 Bootprom version 3.1 is released May 28, 1990, coprighted by FORCE Computers Gmbh
 *
 * Bootprom PIT setup sequence - To start VMEPROM, the rotary switches must both be set to 'F' (PI/T #1 port A)
 * --------------------------------------------------------------------------------------------------------------
 * :pit1 Reg 10 -> ff PAAR - read port A of PIT1 without side effects

 * :pit2 Reg 10 -> 36 PBAR - read port B of PIT2 without side effects
 * :pit2 Reg 10 <- 00 TCR -  PC3/PC7 is I/O, PC2 is I/O, CLK and x32 prescaler, Preload timer on 0 (zero), Timer disabled
 * :pit2 Reg 13 <- ff CPRH - Counter Preload Regsiter High
 * :pit2 Reg 14 <- ff CPRM - Counter Preload Regsiter Mid
 * :pit2 Reg 15 <- ff CPRL - Counter Preload Regsiter Low
 * :pit2 Reg 10 <- 01 TCR -  PC3/PC7 is I/O, PC2 is I/O, CLK and x32 prescaler, Preload timer on 0 (zero), Timer enabled
 * :pit2 Reg 10 <- 00 TCR -  PC3/PC7 is I/O, PC2 is I/O, CLK and x32 prescaler, Preload timer on 0 (zero), Timer disabled
 * :pit2 Reg    -> ff CNTRH - Current value of timer high byte
 * :pit2 Reg    -> b3 CNTRM - Current value of timer mid byte
 * :pit2 Reg    -> 70 CNTRL - Current value of timer low byte

 * :pit1 Reg 10 <- e0 TCR - PC3 is TOUT, PC7 is I/O, Interrupts enabled, PC2 is I/O, CLK and x32 prescaler, Preload timer on 0 (zero), Timer disabled
 * :pit1 Reg 00 <- 20 PGCR -
 * :pit1 Reg 01 <- 08 PSRR -
 * :pit1 Reg 06 <- c0 PACR -
 * :pit1 Reg 02 <- 00 PADDR -
 * :pit1 Reg 07 <- c0 PBCR -
 * :pit1 Reg 09 <- 00 PBDR -
 * :pit1 Reg 03 <- 00 PBDDR -
 * :pit1 Reg 0c <- 17 PCDR -
 * :pit1 Reg 04 <- 17 PCDDR -
 * :pit1 Reg 15 <- e2 CPRL -
 * :pit1 Reg 14 <- 04 CPRM -
 * :pit1 Reg 13 <- 00 CPRH -
 * :pit1 Reg 11 <- f2 TIVR -
 * :pit1 Reg 10 <- 41 TCR - PC3 is SQW at TOUT, PC7 is I/O, PC2 is I/O, CLK and x32 prescaler, Preload timer on 0 (zero), Timer enabled

 * :pit2 Reg 10 <- e2 PC3 is TOUT, PC7 is I/O, Interrupts enabled, PC2 is timer enable/disable, CLK and prescaler, Preload timer on 0 (zero), Timer disabled
 * :pit2 Reg 00 <- 1e PGCR
 * :pit2 Reg 01 <- 08 PSRR
 * :pit2 Reg 06 <- c0 PACR
 * :pit2 Reg 02 <- 00 PADDR
 * :pit2 Reg 07 <- c0 PBCR
 * :pit2 Reg 03 <- 00 PBDDR
 * :pit2 Reg 04 <- 00 PCDDR
 * :pit2 Reg 15 <- 08 CPRL
 * :pit2 Reg 14 <- 00 CPRM
 * :pit2 Reg 13 <- 00 CPRH
 * :pit2 Reg 11 <- f3 TIVR
 * :pit2 Reg 10 <- e3 PC3 is TOUT, PC7 is I/O, Interrupts enabled, PC2 is timer enable/disable, CLK and prescaler, Preload timer on 0 (zero), Timer enabled

 * DUSCC #1 channel A setup 1 sequence FGA-002 firmware (polled i/o)
 *  A Reg 0f <- 00 - reset Tx Command
 *  A Reg 0f <- 40 - reset Rx Command
 *  A Reg 00 <- 07 - Async mode
 *  A Reg 01 <- 38 - Normal polled or interrupt mode, no DMA
 *  A Reg 04 <- 7f - Tx 8 bits, CTS and RTS, 1 STOP bit
 *  A Reg 06 <- 1b - Rx RTS, 8 bits, no DCD, no parity
 *  A Reg 05 <- 3d - Tx BRG 9600 (assuming a 14.7456 crystal)
 *  A Reg 07 <- 2d - Rx BRG 9600 (assuming a 14.7456 crystal)
 *  A Reg 0e <- 27 - TRxC = RxCLK 1x, RTxC is input, RTS, GPO2, crystal oscillator connected to X2
 *  A Reg 0b <- f1 - RTS low, OUT1 = OUT2 = high, RxRdy asserted on FIFO not empty
 *                   TxRdy asserted on FIFO not empty, Same Tx Residual Character Length as for REG_TPR
 *  A Reg 0f <- 00 - reset Tx Command
 *  A Reg 0f <- 40 - reset Rx Command
 *  A Reg 0f <- 02 - enable Tx Command
 *  A Reg 0f <- 42 - enable Rx Command
 *--- end of setup sequence ---
 *  loop:
 *    read  <- REG_GSR
 *  until something needs attention
 *
 * DUSCC #1 channel A & B setup 2 sequence CPU-33 VMEPROM with PDOS (interrupt driven i/o)
 *  A Reg 0f <- 00 - reset Tx Command
 *  A Reg 0f <- 40 - reset Rx Command
 *  A Reg 00 <- 07 - Async mode
 *  A Reg 01 <- 38 - Normal polled or interrupt mode, no DMA
 *  A Reg 04 <- 7f - Tx 8 bits, CTS and RTS, 1 STOP bit
 *  A Reg 06 <- 1b - Rx RTS, 8 bits, no DCD, no parity
 *  A Reg 0b <- e1 - RTS low, OUT1 = OUT2 = high, RxRdy asserted on FIFO not empty
 *                    TxRdy asserted on FIFO not full, Same Tx Residual Character Length as for REG_TPR
 *  A Reg 0e <- 27 - TRxC = RxCLK 1x, RTxC is input, RTS, GPO2, crystal oscillator connected to X2
 *  A Reg 07 <- 2d - Rx BRG 9600 (assuming a 14.7456 crystal)
 *  A Reg 05 <- 3d - Tx BRG 9600 (assuming a 14.7456 crystal)
 *  A Reg 1c <- 10 - RXRDY interrupts enabled
 *  A Reg 1e <- 1c - Interrupt vector
 *  A Reg 1f <- 83 - Interleaved prio with A high, no status in vector, enable interrupts for A & B channel
 *  A Reg 0f <- 00 - reset Tx Command
 *  A Reg 0f <- 02 - enable Tx Command
 *  A Reg 0f <- 40 - reset Rx Command
 *  A Reg 0f <- 42 - enable Rx Command
 * Repeated setup for channel B
 *
 * FGA setup
void fga002_device::do_fga002reg_icrtim0_w(uint8_t)(00)
void fga002_device::do_fga002reg_ctl3_w(uint8_t)(0c)
void fga002_device::do_fga002reg_icrlocal0_w(uint8_t)(00)
void fga002_device::do_fga002reg_icrlocal1_w(uint8_t)(00)
void fga002_device::do_fga002reg_icrlocal2_w(uint8_t)(00)
void fga002_device::do_fga002reg_icrlocal3_w(uint8_t)(00)
void fga002_device::do_fga002reg_icrlocal4_w(uint8_t)(00)
void fga002_device::do_fga002reg_icrlocal5_w(uint8_t)(00)
void fga002_device::do_fga002reg_icrlocal6_w(uint8_t)(00)
void fga002_device::do_fga002reg_icrlocal7_w(uint8_t)(00)

 *  A Reg 10 <- 20
uint8_t duscc_channel::do_dusccreg_rxfifo_r()
 - RX rp:0 wp:1 sz:4
 - RX reading out data:6e 'n'

uint8_t duscc_channel::do_dusccreg_rxfifo_r()
 - RX rp:1 wp:2 sz:4
 - RX reading out data:6e 'n'
 *  A Reg 10 <- 6e
 *  A Reg 10 <- 0d
 *  A Reg 10 <- 0a
void fga002_device::do_fga002reg_icrlocal2_w(uint8_t)(4d)
void fga002_device::do_fga002reg_icrlocal4_w(uint8_t)(1c)
 *  A Reg 0f <- 00
 *  A Reg 0f <- 40
 *  A Reg 00 <- 07
 *  A Reg 01 <- 38
 *  A Reg 04 <- 7f
 *  A Reg 06 <- 1b
 *  A Reg 0b <- e1
 *  A Reg 0e <- 27
 *  A Reg 07 <- 2d
 *  A Reg 05 <- 3d
 *  A Reg 1c <- 10
void duscc_channel::do_dusccreg_ier_w(uint8_t)(10)
- RXRDY interrupts enabled
:duscc void duscc_device::check_interrupts()()
:duscc virtual int duscc_device::z80daisy_irq_state() A:[00][00][00][00] B:[00][00][00][00] Interrupt State 00
void fga002_device::trigger_interrupt(uint8_t)(34)
Interrupt Level 4, caused by ICR 1c with vector 34
void fga002_device::check_interrupts()()
:fga002 virtual int fga002_device::z80daisy_irq_state() Level 0-7:[00][00] [00][00] [01][00] [00][00]
void fccpu30_state::fga_irq_callback(int)(01)
void fga002_device::check_interrupts()()
 */

/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT COMPANY                  FULLNAME          FLAGS */
COMP (1990, fccpu30,      0,      0,       fccpu30,        fccpu30, driver_device, 0,   "Force Computers Gmbh",   "SYS68K/CPU-30", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
