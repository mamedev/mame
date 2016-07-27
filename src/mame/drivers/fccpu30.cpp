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
 * Interrupt sources MVME
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
 * DMAC Channel Assignments
 * ----------------------------------------------------------
 * Channel         MVME147
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

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
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

	DECLARE_READ32_MEMBER (bootvect_r);
	DECLARE_WRITE32_MEMBER (bootvect_w);

	/* Rotary switch PIT input */
	DECLARE_READ8_MEMBER (rotary_rd);
	DECLARE_READ8_MEMBER (board_mem_id_rd);

	/* VME bus accesses */
	//DECLARE_READ16_MEMBER (vme_a24_r);
	//DECLARE_WRITE16_MEMBER (vme_a24_w);
	//DECLARE_READ16_MEMBER (vme_a16_r);
	//DECLARE_WRITE16_MEMBER (vme_a16_w);
	virtual void machine_start () override;
	virtual void machine_reset () override;
protected:

private:
	required_device<cpu_device> m_maincpu;
	required_device<duscc68562_device> m_dusccterm;

	required_device<pit68230_device> m_pit1;
	required_device<pit68230_device> m_pit2;

	required_device<fga002_device> m_fga002;

	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	UINT32  *m_sysrom;
	UINT32  m_sysram[2];
};

static ADDRESS_MAP_START (fccpu30_mem, AS_PROGRAM, 32, fccpu30_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x00000000, 0x00000007) AM_ROM AM_READ  (bootvect_r)   /* ROM mirror just during reset */
	AM_RANGE (0x00000000, 0x00000007) AM_RAM AM_WRITE (bootvect_w)   /* After first write we act as RAM */
	AM_RANGE (0x00000008, 0x003fffff) AM_RAM /* 4 Mb RAM */
	AM_RANGE (0xff000000, 0xff7fffff) AM_ROM AM_REGION("roms", 0x000000)
	AM_RANGE (0xff802000, 0xff8021ff) AM_DEVREADWRITE8("duscc", duscc68562_device, read, write, 0xffffffff) /* Port 1&2 - Dual serial port DUSCC   */
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
	m_sysrom = (UINT32*)(memregion ("roms")->base () + 0x800000);
}

void fccpu30_state::machine_reset ()
{
	LOG(("--->%s\n", FUNCNAME));

	/* Reset pointer to bootvector in ROM for bootvector handler bootvect_r */
	if (m_sysrom == &m_sysram[0]) /* Condition needed because memory map is not setup first time */
		m_sysrom = (UINT32*)(memregion ("roms")->base () + 0x800000);
}

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0xff800000 to 0x0 at reset*/
READ32_MEMBER (fccpu30_state::bootvect_r){
	return m_sysrom[offset];
}

WRITE32_MEMBER (fccpu30_state::bootvect_w){
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
READ8_MEMBER (fccpu30_state::rotary_rd){
	LOG(("%s\n", FUNCNAME));
	return 0xff; // TODO: make this configurable from commandline or artwork
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
READ8_MEMBER (fccpu30_state::board_mem_id_rd){
	LOG(("%s\n", FUNCNAME));
	//  return 0x6A; // CPU-30 R4 with 4Mb of shared RAM. TODO: make this configurable from commandline or artwork
	//  return 0x57; // blankt 53 56
	//      return 0x36; // CPU-26 1Mb 36 MHz
	//      return 0x35; // CPU-26 1Mb 36 MHz
			return 0x36; // CPU-26 1Mb 36 MHz
}

#if 0
/* Dummy VME access methods until the VME bus device is ready for use */
READ16_MEMBER (fccpu30_state::vme_a24_r){
	LOG (logerror ("vme_a24_r\n"));
	return (UINT16) 0;
}

WRITE16_MEMBER (fccpu30_state::vme_a24_w){
	LOG (logerror ("vme_a24_w\n"));
}

READ16_MEMBER (fccpu30_state::vme_a16_r){
	LOG (logerror ("vme_16_r\n"));
	return (UINT16) 0;
}

WRITE16_MEMBER (fccpu30_state::vme_a16_w){
	LOG (logerror ("vme_a16_w\n"));
}
#endif

/*
 * Machine configuration
 */
static MACHINE_CONFIG_START (fccpu30, fccpu30_state)
	/* basic machine hardware */
	MCFG_CPU_ADD ("maincpu", M68030, XTAL_25MHz)
	MCFG_CPU_PROGRAM_MAP (fccpu30_mem)
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Terminal Port config */
	MCFG_DUSCC68562_ADD("duscc", DUSCC_CLOCK, 0, 0, 0, 0 )
	MCFG_DUSCC_OUT_TXDA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_txd))
	MCFG_DUSCC_OUT_DTRA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_dtr))
	MCFG_DUSCC_OUT_RTSA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD ("rs232trm", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("duscc", duscc68562_device, rxa_w))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("duscc", duscc68562_device, ctsa_w))

	MCFG_DUSCC68562_ADD("duscc2", DUSCC_CLOCK, 0, 0, 0, 0 )

	/* PIT Parallel Interface and Timer device, assuming strapped for on board clock */
	MCFG_DEVICE_ADD ("pit1", PIT68230, XTAL_16MHz / 2) // The PIT clock is not verified on schema but reversed from behaviour
	MCFG_PIT68230_PA_INPUT_CB(READ8(fccpu30_state, rotary_rd))
	MCFG_DEVICE_ADD ("pit2", PIT68230, XTAL_16MHz / 2) // Th PIT clock is not verified on schema but reversed from behaviour
	MCFG_PIT68230_PB_INPUT_CB(READ8(fccpu30_state, board_mem_id_rd))

	/* FGA-002, Force Gate Array */
	MCFG_FGA002_ADD("fga002", 0)
MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (fccpu30)
	ROM_REGION32_BE(0x900000, "roms", 0)

// Boots with Board ID set to: 0x36
	ROM_LOAD16_BYTE("CPU33LO.BIN",  0x000001, 0x40000, CRC (49895fdf) SHA1 (733abd144c95225a2faf920490e31df2a27f8e03))
	ROM_LOAD16_BYTE("CPU33UP.BIN",  0x000000, 0x40000, CRC (cfe75e94) SHA1 (d40e0635a48607be25f7c58c74b53b7e58fe735d))

// Boots with Board ID set to: 0x36 (no display yet)
#if 0
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
 * Bootprom PIT setup sequence
 *  0a 00 <- read port A without side effects
 *  0b 00 <- read port B without side effects
 *  10 00 -> TCR - Timer Control register: Disable timer
 *  13 ff -> CPRH - Counter Preload Regsiter High
 *  14 ff -> CPRM - Counter Preload Regsiter Mid
 *  15 ff -> CPRL - Counter Preload Regsiter Low
 *  10 01 -> TCR - Timer Control register: Enable timer
 * ------ init ends -------- clock: 4217
 *
 * To start VMEPROM, the rotary switches must both be set to 'F' (PI/T #1 port A)
 *
 * ------ next config -------- clock: 1964222
 *  10 00 -> TCR - Timer Control register: Disable timer
 *  17 00 -> CRH - Counter Register High
 *  18 00 -> CRM - Counter Register Medium
 *  19 00 -> CRL - Counter Register Low
 *
 * DUSCC #1 channel A setup sequence
 *  0f 00 -> REG_CCR    - reset Tx Command
 *  0f 40 -> REG_CCR    - reset Rx Command
 *  00 07 -> REG_CMR1   - Async mode
 *  01 38 -> REG_CMR2   - Normal polled or interrupt mode, no DMA
 *  04 7f -> REG_TPR    - Tx 8 bits, CTS and RTS, 1 STOP bit
 *  06 1b -> REG_RPR    - Rx RTS, 8 bits, no DCD, no parity
 *  05 3d -> REG_TTR    - Tx BRG 9600 (assuming a 14.7456 crystal)
 *  07 2d -> REG_RTR    - Rx BRG 9600 (assuming a 14.7456 crystal)
 *  0e 27 -> REG_PCR    - TRxC = RxCLK 1x, RTxC is input, RTS, GPO2, crystal oscillator connected to X2
 *  0b f1 -> REG_OMR    - RTS low, OUT1 = OUT2 = high, RxRdy asserted for each character,
 *                        TxRdy asserted on threshold, Same Tx Residual Character Length as for REG_TPR
 *  0f 00 -> REG_CCR    - reset Tx Command
 *  0f 40 -> REG_CCR    - reset Rx Command
 *  0f 02 -> REG_CCR    - enable Tx Command
 *  0f 42 -> REG_CCR    - enable Rx Command
 *--- end of setup sequence ---
 *  loop:
 *    read  <- REG_GSR
 *  until something needs attention
 */

/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT COMPANY                  FULLNAME          FLAGS */
COMP (1990, fccpu30,      0,      0,       fccpu30,        fccpu30, driver_device, 0,   "Force Computers Gmbh",   "SYS68K/CPU-30", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
