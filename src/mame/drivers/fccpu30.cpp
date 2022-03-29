// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Force SYS68K CPU-30 VME SBC drivers
 *
 *  21/05/2016
 *
 * Thanks to Al Kossow and his site http://www.bitsavers.org/ I got the information
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
 * 1981 Force Computers was founded in San Jose, California. Over time a European headquarter was opened
 *      in Munich, Germany, and a Japanese headquarter in Tokyo
 * 1996 Force was acquired by Solectron Corporation in 1996
 * 2004 Force was sold off from Solectron to Motorola
 * 2008 Force was acquired by Emerson as part of the Motorola Embedded Division
 * 2016 Force was acquired by Platinum Equity as part of the Emerson Network Power Division
 *
 * Force developed and produced VME board products based on SPARC, Pentium, PowerPC and 68K.
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
 *      -  Floppy disk interface on P2 connector (FCD37C65C)
 *      -  Four serial ports (DUSCC SCN68562 x 2)
 *      -  20 bit digital i/o for user applications( 2 x 68230 PI/T )
 *      -  Real-Time Clock with interrupt (72423)
 *      -  4-level requester, 7-level interrupter, and 7-level interrupt handler for VMEbus (VMEchip ASIC)
 *
 * NOTE: This driver currently mimics the CPU-30xyz configuration: 16MHz, 4Mb RAM, no parity, no ethernet (See TODO)
 *
 * Address Map
 * --------------------------------------------------------------------------
 *  Range                   Desccription
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
#include "bus/vme/vme.h"
#include "bus/vme/vme_fcisio.h"
#include "bus/vme/vme_fcscsi.h"
#include "machine/msm6242.h"
#include "machine/ram.h" // For variants that only differs in amount of RAM
#include "machine/scnxx562.h"
#include "machine/68230pit.h"
#include "machine/fga002.h"
#include "machine/nvram.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
//#include "machine/timekpr.h"

//#define LOG_GENERAL (1U <<  0)
#define LOG_SETUP   (1U <<  1)
#define LOG_READ    (1U <<  2)
#define LOG_INT     (1U <<  3)
#define LOG_INIT    (1U <<  4)

//#define VERBOSE (LOG_GENERAL | LOG_SETUP)
//#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP,  __VA_ARGS__)
#define LOGR(...)     LOGMASKED(LOG_READ,   __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,    __VA_ARGS__)
#define LOGINIT(...)  LOGMASKED(LOG_INIT,   __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define DUSCC_CLOCK XTAL(14'745'600) /* Verified */

class cpu30_state : public driver_device
{
public:
cpu30_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device (mconfig, type, tag)
		, m_maincpu (*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_dusccterm(*this, "duscc")
		, m_pit1 (*this, "pit1")
		, m_pit2 (*this, "pit2")
		, m_fga002 (*this, "fga002")
		, m_rtc (*this, "rtc")
		, m_board_id(0x50)
	{
	}

	void cpu30(machine_config &config);
	void cpu30x(machine_config &config);
	void cpu30zbe(machine_config &config);
	void cpu30be8(machine_config &config);
	void cpu30za(machine_config &config);
	void cpu30lite4(machine_config &config);
	void cpu30xa(machine_config &config);
	void cpu33(machine_config &config);
	void cpu30lite8(machine_config &config);
	void cpu30be16(machine_config &config);

	void init_cpu30x();
	void init_cpu30xa();
	void init_cpu30za();
	void init_cpu30zbe();
	void init_cpu30be8();
	void init_cpu30be16();
	void init_cpu30lite4();
	void init_cpu30lite8();
	void init_cpu33();

private:
	void fdc_w(offs_t offset, uint8_t data);
	uint8_t fdc_r(offs_t offset);
	void scsi_w(offs_t offset, uint8_t data);
	uint8_t scsi_r(offs_t offset);
	uint8_t slot1_status_r();
	uint32_t bootvect_r(offs_t offset);
	void bootvect_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	/* Interrupt  support */
	void cpu_space_map(address_map &map);
	DECLARE_WRITE_LINE_MEMBER(fga_irq_callback);
	uint8_t fga_irq_state = 0;
	//  int fga_irq_vector = 0;
	int fga_irq_level = 0;

	/* Rotary switch PIT input */
	uint8_t rotary_rd();
	uint8_t flop_dmac_r();
	void flop_dmac_w(uint8_t data);
	uint8_t pit1c_r();
	void pit1c_w(uint8_t data);
	uint8_t pit2a_r();
	void pit2a_w(uint8_t data);
	uint8_t board_mem_id_rd();
	uint8_t pit2c_r();
	void pit2c_w(uint8_t data);

	/* VME bus accesses */
	//uint16_t vme_a24_r();
	//void vme_a24_w(uint16_t data);
	//uint16_t vme_a16_r();
	//void vme_a16_w(uint16_t data);
	virtual void machine_start () override;
	virtual void machine_reset () override;

	void cpu30_mem(address_map &map);

	required_device<m68000_base_device> m_maincpu;
	required_device<ram_device> m_ram;

	required_device<duscc68562_device> m_dusccterm;

	required_device<pit68230_device> m_pit1;
	required_device<pit68230_device> m_pit2;

	required_device<fga002_device> m_fga002;

	required_device<rtc72423_device> m_rtc;

	// Helper functions
	void update_irq_to_maincpu();

	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	uint32_t  *m_sysrom = nullptr;
	uint32_t  m_sysram[2]{};

	uint8_t m_board_id;
};

void cpu30_state::cpu30_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x00000007).ram().w(FUNC(cpu30_state::bootvect_w));   /* After first write we act as RAM */
	map(0x00000000, 0x00000007).rom().r(FUNC(cpu30_state::bootvect_r));   /* ROM mirror just during reset */
//  map(0x00000008, 0x003fffff).ram(); /* RAM  installed in machine start */
	map(0xff000000, 0xff7fffff).rom().region("roms", 0x000000);
	map(0xff800c00, 0xff800dff).rw(m_pit1, FUNC(pit68230_device::read), FUNC(pit68230_device::write));
	map(0xff800e00, 0xff800fff).rw(m_pit2, FUNC(pit68230_device::read), FUNC(pit68230_device::write));
	map(0xff802000, 0xff8021ff).rw(m_dusccterm, FUNC(duscc68562_device::read), FUNC(duscc68562_device::write)); /* Port 1&2 - Dual serial port DUSCC   */
	map(0xff802200, 0xff8023ff).rw("duscc2", FUNC(duscc68562_device::read), FUNC(duscc68562_device::write)); /* Port 3&4 - Dual serial port DUSCC   */
	map(0xff803000, 0xff8031ff).rw(m_rtc, FUNC(rtc72423_device::read), FUNC(rtc72423_device::write));
//  map(0xff803400, 0xff8035ff).rw("scsi", FUNC(mb87033_device::read), FUNC(mb87033_device::write)); /* TODO: implement MB87344 SCSI device */
	map(0xff803400, 0xff8035ff).rw(FUNC(cpu30_state::scsi_r), FUNC(cpu30_state::scsi_w)).umask32(0x000000ff); /* mock driver to log calls to device */
//  map(0xff803800, 0xff80397f).rw("fdc", FUNC(wd37c65c_device::read), FUNC(wd37c65c_device::write)); /* TODO: implement WD3/C65C fdc controller */
	map(0xff803800, 0xff80397f).rw(FUNC(cpu30_state::fdc_r), FUNC(cpu30_state::fdc_w)).umask32(0x000000ff); /* mock driver to log calls to device */
	map(0xff803980, 0xff8039ff).r(FUNC(cpu30_state::slot1_status_r)).umask32(0x000000ff);
	map(0xffc00000, 0xffcfffff).ram().share("nvram"); /* On-board SRAM with battery backup (nvram) */
	map(0xffd00000, 0xffdfffff).rw(m_fga002, FUNC(fga002_device::read), FUNC(fga002_device::write));  /* FGA-002 Force Gate Array */
	map(0xffe00000, 0xffefffff).rom().region("roms", 0x800000);

	//map(0x100000, 0xfeffff).rw(FUNC(cpu30_state::vme_a24_r), FUNC(cpu30_state::vme_a24_w)); /* VMEbus Rev B addresses (24 bits) - not verified */
	//map(0xff0000, 0xffffff).rw(FUNC(cpu30_state::vme_a16_r), FUNC(cpu30_state::vme_a16_w)); /* VMEbus Rev B addresses (16 bits) - not verified */
}

/* Input ports */
static INPUT_PORTS_START (cpu30)
INPUT_PORTS_END

/* Start it up */
void cpu30_state::machine_start ()
{
	LOGINIT("%s\n", FUNCNAME);

	save_pointer (NAME (m_sysrom), sizeof(m_sysrom));
	save_pointer (NAME (m_sysram), sizeof(m_sysram));

	/* setup ram */
	m_maincpu->space(AS_PROGRAM).install_ram(0x08, m_ram->size() - 1, m_ram->pointer());

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (uint32_t*)(memregion ("roms")->base () + 0x800000);
}

void cpu30_state::machine_reset ()
{
	LOGINIT("%s\n", FUNCNAME);

	/* Reset pointer to bootvector in ROM for bootvector handler bootvect_r */
	if (m_sysrom == &m_sysram[0]) /* Condition needed because memory map is not setup first time */
		m_sysrom = (uint32_t*)(memregion ("roms")->base () + 0x800000);
}

/*                                                                              setup board ID */
void cpu30_state::init_cpu30x()      { LOGINIT("%s\n", FUNCNAME); m_board_id = 0x50; }
void cpu30_state::init_cpu30xa()     { LOGINIT("%s\n", FUNCNAME); m_board_id = 0x50; }
void cpu30_state::init_cpu30za()     { LOGINIT("%s\n", FUNCNAME); m_board_id = 0x50; }
void cpu30_state::init_cpu30zbe()    { LOGINIT("%s\n", FUNCNAME); m_board_id = 0x50; }
void cpu30_state::init_cpu30be8()    { LOGINIT("%s\n", FUNCNAME); m_board_id = 0x50; }
void cpu30_state::init_cpu30be16()   { LOGINIT("%s\n", FUNCNAME); m_board_id = 0x50; }
void cpu30_state::init_cpu30lite4()  { LOGINIT("%s\n", FUNCNAME); m_board_id = 0x50; }
void cpu30_state::init_cpu30lite8()  { LOGINIT("%s\n", FUNCNAME); m_board_id = 0x50; }
void cpu30_state::init_cpu33()       { LOGINIT("%s\n", FUNCNAME); m_board_id = 0x68; } // 0x60 skips FGA prompt

/* Mock FDC driver */
uint8_t cpu30_state::fdc_r(offs_t offset){
	LOG("%s\n * FDC read Offset: %04x\n", FUNCNAME, offset);
	return 1;
}

void cpu30_state::fdc_w(offs_t offset, uint8_t data){
	LOG("%s\n * FDC write Offset: %04x Data: %02x\n", FUNCNAME, offset, data);
}

/* Mock SCSI driver */
uint8_t cpu30_state::scsi_r(offs_t offset){
	LOG("%s\n * SCSI read Offset: %04x\n", FUNCNAME, offset);
	return 1;
}

void cpu30_state::scsi_w(offs_t offset, uint8_t data){
	LOG("%s\n * SCSI write Offset: %04x Data: %02x\n", FUNCNAME, offset, data);
}

/* 1 = board is in slot 1, 0 = board is NOT in slot 1 */
uint8_t cpu30_state::slot1_status_r(){
	LOG("%s\n", FUNCNAME);
	return 1;
}

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0xff800000 to 0x0 at reset*/
uint32_t cpu30_state::bootvect_r(offs_t offset){
	LOG("%s\n", FUNCNAME);
	return m_sysrom[offset];
}

void cpu30_state::bootvect_w(offs_t offset, uint32_t data, uint32_t mem_mask){
	LOG("%s\n", FUNCNAME);
	m_sysram[offset % std::size(m_sysram)] &= ~mem_mask;
	m_sysram[offset % std::size(m_sysram)] |= (data & mem_mask);
	m_sysrom = &m_sysram[0]; // redirect all upcoming accesses to masking RAM until reset.
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
uint8_t cpu30_state::rotary_rd(){
	LOG("%s\n", FUNCNAME);
	return 0xff; // TODO: make this configurable from commandline or artwork
}

// PIT#1 Port B TODO: implement floppy and dma control
uint8_t cpu30_state::flop_dmac_r(){
	LOG("%s\n", FUNCNAME);
	return 0xff;
}

void cpu30_state::flop_dmac_w(uint8_t data){
	LOG("%s(%02x)\n", FUNCNAME, data);
}

#define FPCP_SENSE 0x40 /* Port C bit 6 is low if a Floating Point Co Processor is installed */
// PIT#1 Port C TODO: implement timer+port interrupts
// TODO: Connect PC0, PC1, PC4 and PC7 to B5 and/or P2 connector
uint8_t cpu30_state::pit1c_r(){
	LOG("%s\n", FUNCNAME);
	m_maincpu->set_fpu_enable(1);    // Lets assume the FPCP is always installed ( which is default for 68030 atm )
	return 0xff & ~FPCP_SENSE; // Should really be command line for the edge cases...
}

void cpu30_state::pit1c_w(uint8_t data){
	LOG("%s(%02x)\n", FUNCNAME, data);
}

// PIT#2 Port A TODO: Connect to B5 and /or P2 connector
uint8_t cpu30_state::pit2a_r(){
	LOG("%s\n", FUNCNAME);
	logerror("Unsupported user i/o on PIT2 port A detected\n");
	return 0xff;
}

void cpu30_state::pit2a_w(uint8_t data){
	LOG("%s(%02x)\n", FUNCNAME, data);
	logerror("Unsupported user i/o on PIT2 port A detected\n");
}

/*
 * PI/T #2 Factory settings
 * B2 B1 B0 Shared Memory Size - From these lines, the on-board Shared RAM capacity can be read in by software.
 *  0  0  0 32 Mb
 *  0  0  1 16 Mb
 *  0  1  0  8 Mb
 *  0  1  1  4 Mb
 * 1 x x  Reserved
 * B3-B7 Board ID(s) -  From these lines, the CPU board identification number can be read in by
 * 0 1 0 1 0 CPU-30 R4  software. Every CPU board has a unique number. Different versions of
 * (fill in more)       one CPU board (i.e. different speeds, capacity of memory, or modules)
 *                      contain the same identification number. In the case of the CPU-30 R4, the
 *                      number is 0x0a
 *
 * The speed of the board is measured by looping some instructions and measure the time it takes with a timer
 * Currently this doesn't work so the wrong speed is displayed on the screen. To fix this timing needs to be more exact.
 * Speed measure subroutine is at address 0xffe033c4 in the FGA-002 firmware with CRC (faa38972) (for example)
 */
uint8_t cpu30_state::board_mem_id_rd()
{
	int sz;
	LOG("%s\n", FUNCNAME);
	switch (m_ram->size())
	{
	case (1024 * 1024 * 32): sz = 0; break;
	case (1024 * 1024 * 16): sz = 1; break;
	case (1024 * 1024 * 8):  sz = 2; break;
	case (1024 * 1024 * 4):  sz = 3; break;
	case (1024 * 1024 * 2):  sz = 4; break;
	case (1024 * 1024 * 1):  sz = 5; break;
	case (1024 * 512  * 1):  sz = 6; break;
	case (1024 * 256  * 1):  sz = 7; break;
	default:
		logerror("No supported RAM size, telling VMEPROM 4Mb\n");
		sz = 4;
	}
	//  return 0x6A; // CPU-30 R4 with 4Mb of shared RAM. TODO: make this configurable from commandline or artwork
	//  return 0x57; // blankt 53 56
	//      return 0x36; // CPU-26 1Mb 36 MHz
	//      return 0x35; // CPU-26 1Mb 36 MHz
	// calculate response from m_ram (default 4M or command line -ramsize) Bit 2 is reserved and always 0 on CPU-30
	// return 0x6B;// CPU-33, 4Mb, 36MHz p4
	// return 0x6B;// CPU-33, 4Mb, 36MHz p4
	// return 0x99;// CPU-26, 4Mb, 36MHz p4
	// return 0x33;// CPU-26, 4Mb, 36MHz p4
	// return 0x37;// CPU-26, 4Mb, 36MHz p4
	// return 0x35;// CPU-26, 1Mb, 36MHz p4
	// return 0x36;// CPU-26, 1Mb, 36MHz p4
	// return 0x55;// CPU-30, 1Mb, 36MHz
	// return 0x56;// CPU-30, 1Mb, 36MHz
	// return 0x53;// CPU-30, 4Mb, 36MHz
	// return 0x57;// CPU-30, 4Mb, 36MHz
	//  return 0x48 + sz;// none
	//  return 0x38 + sz;// none
	//  return 0x28 + sz;// none
	//  return 0x18 + sz;// CPU-23 p1
	//  return 0x10 + sz;// CPU-22 p4
	//  return 0x20 + sz;// p1: Wait until hard disk is up to speed
	//  printf("SIZE:%02x\n", sz);
	LOG("- Board ID:%02x Size:%02x\n", m_board_id, sz);
	return m_board_id + sz;
}

// PIT#2 Port C TODO: implement timer interrupt, DMA i/o, memory control and Hardware ID
uint8_t cpu30_state::pit2c_r(){
	LOG("%s\n", FUNCNAME);
	return 0xfe;
}

void cpu30_state::pit2c_w(uint8_t data){
	LOG("%s(%02x)\n", FUNCNAME, data);
}

#if 0
/* Dummy VME access methods until the VME bus device is ready for use */
uint16_t cpu30_state::vme_a24_r(){
	LOG("%s\n", FUNCNAME);
	return (uint16_t) 0;
}

void cpu30_state::vme_a24_w(uint16_t data){
	LOG("%s(%02x)\n", FUNCNAME, data);
}

uint16_t cpu30_state::vme_a16_r(){
	LOG("%s\n", FUNCNAME);
	return (uint16_t) 0;
}

void cpu30_state::vme_a16_w(uint16_t data){
	LOG("%s(%02x)\n", FUNCNAME, data);
}
#endif

WRITE_LINE_MEMBER(cpu30_state::fga_irq_callback)
{
	LOGINT("%s(%02x)\n", FUNCNAME, state);

	fga_irq_state = state;
	fga_irq_level = m_fga002->get_irq_level();
	LOGINT(" - FGA irq level  %02x\n", fga_irq_level);
	update_irq_to_maincpu();
}

void cpu30_state::update_irq_to_maincpu()
{
	LOGINT("%s()\n", FUNCNAME);
	LOGINT(" - fga_irq_level: %02x\n", fga_irq_level);
	LOGINT(" - fga_irq_state: %02x\n", fga_irq_state);
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

static void fccpu30_vme_cards(device_slot_interface &device)
{
	device.option_add("fcisio", VME_FCISIO1);
	device.option_add("fcscsi", VME_FCSCSI1);
}

/*
 * Machine configuration
 */

void cpu30_state::cpu_space_map(address_map &map)
{
	map(0xfffffff2, 0xffffffff).lr16(NAME([this] (offs_t offset) -> u16 { return m_fga002->iack(); }));
}

void cpu30_state::cpu30(machine_config &config)
{
	/* basic machine hardware */
	M68030(config, m_maincpu, XTAL(25'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &cpu30_state::cpu30_mem);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &cpu30_state::cpu_space_map);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	VME(config, "vme", 0);
	VME_SLOT(config, "slot1", fccpu30_vme_cards, nullptr, 1, "vme");
	/* Terminal Port config */
	/* Force CPU30 series of boards has up to four serial ports, p1-p4, the FGA boot uses p4 as console and subsequent
	   firmware uses p1 as console and in an operating system environment there may be user login shells on the other.

	   In order to use more than just one terminal MAME supports serial socket servers to which it is possible to
	   connect a telnet terminal to. The general syntax to open a socket server from the command prompts is:

	   mame  cpu30 -window -rs232p4 null_modem -bitbngr socket.127.0.0.1:1001

	   At the opening screen, before the board starts to execute code, start up the telnet client and give 127.0.0.1:1001 as host
	   It is also possible to enumerate more than one terminal server in order to have several terminal session attached.

	   mame  cpu30 -window -rs232p4 null_modem -bitbngr1 socket.127.0.0.1:1001 -rs232p1 null_modem -bitbngr2 socket.127.0.0.1:1002

	   Now just start up the telnet clients with 127.0.0.1:1001 and 127.0.0.1:1002 as hosts and you have control of input for each port.
	*/

#define RS232P1_TAG      "rs232p1"
#define RS232P2_TAG      "rs232p2"
#define RS232P3_TAG      "rs232p3"
#define RS232P4_TAG      "rs232p4"

	DUSCC68562(config, m_dusccterm, DUSCC_CLOCK);
	m_dusccterm->configure_channels(0, 0, 0, 0);
	/* Port 1 on Port B */
	m_dusccterm->out_txdb_callback().set(RS232P1_TAG, FUNC(rs232_port_device::write_txd));
	m_dusccterm->out_dtrb_callback().set(RS232P1_TAG, FUNC(rs232_port_device::write_dtr));
	m_dusccterm->out_rtsb_callback().set(RS232P1_TAG, FUNC(rs232_port_device::write_rts));
	/* Port 4 on Port A */
	m_dusccterm->out_txda_callback().set(RS232P4_TAG, FUNC(rs232_port_device::write_txd));
	m_dusccterm->out_dtra_callback().set(RS232P4_TAG, FUNC(rs232_port_device::write_dtr));
	m_dusccterm->out_rtsa_callback().set(RS232P4_TAG, FUNC(rs232_port_device::write_rts));
	/* DUSCC1 interrupt signal REQN is connected to LOCAL IRQ4 of the FGA-002 and level is programmable */
	m_dusccterm->out_int_callback().set(m_fga002, FUNC(fga002_device::lirq4_w));

	duscc68562_device &duscc2(DUSCC68562(config, "duscc2", DUSCC_CLOCK));
	duscc2.configure_channels(0, 0, 0, 0);
	/* Port 2 on Port A */
	duscc2.out_txda_callback().set(RS232P2_TAG, FUNC(rs232_port_device::write_txd));
	duscc2.out_dtra_callback().set(RS232P2_TAG, FUNC(rs232_port_device::write_dtr));
	duscc2.out_rtsa_callback().set(RS232P2_TAG, FUNC(rs232_port_device::write_rts));
	/* Port 3 on Port B */
	duscc2.out_txdb_callback().set(RS232P3_TAG, FUNC(rs232_port_device::write_txd));
	duscc2.out_dtrb_callback().set(RS232P3_TAG, FUNC(rs232_port_device::write_dtr));
	duscc2.out_rtsb_callback().set(RS232P3_TAG, FUNC(rs232_port_device::write_rts));
	/* DUSCC2 interrupt signal REQN is connected to LOCAL IRQ5 of the FGA-002 and level is programmable */
	duscc2.out_int_callback().set(m_fga002, FUNC(fga002_device::lirq5_w));

	rs232_port_device &rs232p1(RS232_PORT(config, RS232P1_TAG, default_rs232_devices, "terminal"));
	rs232p1.rxd_handler().set(m_dusccterm, FUNC(duscc68562_device::rxb_w));
	rs232p1.cts_handler().set(m_dusccterm, FUNC(duscc68562_device::ctsb_w));

	rs232_port_device &rs232p2(RS232_PORT(config, RS232P2_TAG, default_rs232_devices, nullptr));
	rs232p2.rxd_handler().set("duscc2", FUNC(duscc68562_device::rxa_w));
	rs232p2.cts_handler().set("duscc2", FUNC(duscc68562_device::ctsa_w));

	rs232_port_device &rs232p3(RS232_PORT(config, RS232P3_TAG, default_rs232_devices, nullptr));
	rs232p3.rxd_handler().set("duscc2", FUNC(duscc68562_device::rxb_w));
	rs232p3.cts_handler().set("duscc2", FUNC(duscc68562_device::ctsb_w));

	rs232_port_device &rs232p4(RS232_PORT(config, RS232P4_TAG, default_rs232_devices, "terminal"));
	rs232p4.rxd_handler().set(m_dusccterm, FUNC(duscc68562_device::rxa_w));
	rs232p4.cts_handler().set(m_dusccterm, FUNC(duscc68562_device::ctsa_w));

	/* PIT Parallel Interface and Timer device, assumed strapped for on board clock */
	PIT68230(config, m_pit1, XTAL(16'000'000) / 2); // The PIT clock is not verified on schema but reversed from behaviour
	m_pit1->pa_in_callback().set(FUNC(cpu30_state::rotary_rd));
	m_pit1->pb_in_callback().set(FUNC(cpu30_state::flop_dmac_r));
	m_pit1->pb_out_callback().set(FUNC(cpu30_state::flop_dmac_w));
	m_pit1->pc_in_callback().set(FUNC(cpu30_state::pit1c_r));
	m_pit1->pc_out_callback().set(FUNC(cpu30_state::pit1c_w));
//  m_pit1->timer_irq_callback().set(m_fga002, FUNC(fga002_device::lirq2_w)); // The timer interrupt seems to silence the terminal interrupt, needs invectigation

	PIT68230(config, m_pit2, XTAL(16'000'000) / 2); // Th PIT clock is not verified on schema but reversed from behaviour
	m_pit2->pb_in_callback().set(FUNC(cpu30_state::board_mem_id_rd));
	m_pit2->pa_in_callback().set(FUNC(cpu30_state::pit2a_r));
	m_pit2->pa_out_callback().set(FUNC(cpu30_state::pit2a_w));
	m_pit2->pc_in_callback().set(FUNC(cpu30_state::pit2c_r));
	m_pit2->pc_out_callback().set(FUNC(cpu30_state::pit2c_w));
//  m_pit2->timer_irq_callback().set(m_fga002, FUNC(fga002_device::lirq3_w)); // The timer interrupt seems to silence the terminal interrupt, needs invectigation

	/* FGA-002, Force Gate Array */
	FGA002(config, m_fga002, 0);
	m_fga002->out_int().set(FUNC(cpu30_state::fga_irq_callback));
	m_fga002->liack4().set("duscc",  FUNC(duscc_device::iack));
	m_fga002->liack5().set("duscc2", FUNC(duscc_device::iack));

	// RTC
	RTC72423(config, m_rtc, XTAL(32'768)); // Fake crystal value, the 72423 uses it own internal crystal
	m_rtc->out_int_handler().set(m_fga002, FUNC(fga002_device::lirq0_w));

	// dual ported ram
	RAM(config, m_ram).set_default_size("4M").set_extra_options("8M, 16M, 32M");
}

/* SYS68K/CPU-30X Part No.1 01300: 16.7 MHz 68030 based CPU board with 68882 FPCP, DMAC, 1 Mbyte Dual Ported RAM capacity and VMEPROM. */
void cpu30_state::cpu30x(machine_config &config)
{
	cpu30(config);
	m_maincpu->set_clock(XTAL(16'777'216)); /* 16.7 MHz  from description, crystal needs verification */

//  config.device_remove("");

	// dual ported ram
	m_ram->set_default_size("1M").set_extra_options("1M, 2M, 4M");
}

/* SYS68K/CPU-30XA Part No.1 01301: 20.0 MHz 68030 based CPU board with 68882 FPCP, DMAC, 1 Mbyte Dual Ported RAM capacity and VMEPROM. Documentation included.*/
void cpu30_state::cpu30xa(machine_config &config)
{
	cpu30x(config);
	m_maincpu->set_clock(XTAL(20'000'000)); /* 20.0 MHz  from description, crystal needs verification */
}

/* SYS68K/CPU-30ZA Part No.1 01302: 20.0 MHz 68030 based CPU board with 68882 FPCP, DMAC, 4 Mbyte Dual Ported RAM capacity and VMEPROM. Documentation included.*/
void cpu30_state::cpu30za(machine_config &config)
{
	cpu30xa(config);
	m_maincpu->set_clock(XTAL(20'000'000)); /* 20.0 MHz  from description, crystal needs verification */

	// dual ported ram
	m_ram->set_default_size("4M").set_extra_options("1M, 2M, 4M");
}

/* SYS68K/CPU-30ZBE 68030/68882 CPU, 25 MHz,  4 Mbyte shared DRAM, 4 Mbyte Flash, SCSI, Ethernet, Floppy disk, 4 serial I/O ports, 32-bit VMEbus interface */
void cpu30_state::cpu30zbe(machine_config &config)
{
	cpu30za(config);
	m_maincpu->set_clock(XTAL(25'000'000)); /* 25.0 MHz  from description, crystal needs verification */

	// dual ported ram
	m_ram->set_default_size("4M").set_extra_options("256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M");
}

/* SYS68K/CPU-33 */
void cpu30_state::cpu33(machine_config &config)
{
	cpu30zbe(config);
	m_maincpu->set_clock(XTAL(25'000'000)); /* 25.0 MHz  from description, crystal needs verification */

	// dual ported ram
	m_ram->set_default_size("4M").set_extra_options("256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M");
}

/* SYS68K/CPU-30BE/8 68030/68882 CPU, 25 MHz,  8 Mbyte shared DRAM, 4 Mbyte Flash, SCSI, Ethernet, Floppy disk, 4 serial I/O ports, 32-bit VMEbus interface, VMEPROM firmware*/
void cpu30_state::cpu30be8(machine_config &config)
{
	cpu30zbe(config);
	// dual ported ram
	m_ram->set_default_size("8M").set_extra_options("256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M");
}

/* SYS68K/CPU-30BE/16 68030/68882 CPU, 25 MHz, 16 Mbyte shared DRAM, 4 Mbyte Flash, SCSI, Ethernet, Floppy disk, 4 serial I/O ports, 32-bit VMEbus interface, VMEPROM firmware*/
void cpu30_state::cpu30be16(machine_config &config)
{
	cpu30zbe(config);
	// dual ported ram
	m_ram->set_default_size("16M").set_extra_options("256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M");
}

/* SYS68K/CPU-30Lite/4 68030 CPU, 25 MHz, 4 Mbyte shared DRAM, 4 Mbyte Flash, 4 serial ports, 32-bit VMEbus interface, VMEPROM firmware. */
void cpu30_state::cpu30lite4(machine_config &config)
{
	cpu30zbe(config);
// Enable these when added to main config
//  config.device_remove("fpu");
//  config.device_remove("scsi");
//  config.device_remove("eth");
//  config.device_remove("fdc");
	// dual ported ram
	m_ram->set_default_size("4M").set_extra_options("256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M");
}

/* SYS68K/CPU-30Lite/8 68030 CPU, 25 MHz, 4 Mbyte shared DRAM, 8 Mbyte Flash, 4 serial ports, 32-bit VMEbus interface, VMEPROM firmware. */
void cpu30_state::cpu30lite8(machine_config &config)
{
	cpu30lite4(config);
	// dual ported ram
	m_ram->set_default_size("8M").set_extra_options("256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M");
}

/* ROM definitions */
ROM_START (fccpu30) /* This is an original rom dump */
	ROM_REGION32_BE(0x900000, "roms", 0)
// Boots with Board ID set to: 0x36 (FGA002 BOOT on terminal P4, "Wait until harddisk is up to speed " on terminal P1)
	ROM_LOAD32_BYTE("cpu30ll-ubr-v2.1.rom",  0x000003, 0x20000, CRC (a03ebf46) SHA1 (48fa0268cb10e20679c093e02574dbd9925f95d1))
	ROM_LOAD32_BYTE("cpu30lo-ubr-v2.1.rom",  0x000002, 0x20000, CRC (fefa88ed) SHA1 (71a9ad807c0c2da5c6f6a6dc68c73ad8b52f3ea9))
	ROM_LOAD32_BYTE("cpu30up-ubr-v2.1.rom",  0x000001, 0x20000, CRC (dfed1f68) SHA1 (71478a77d5ab5da0fabcd78e69537919b560e3b8))
	ROM_LOAD32_BYTE("cpu30uu-ubr-v2.1.rom",  0x000000, 0x20000, CRC (66e95cc2) SHA1 (acdb468a3a5974295b81271d617de7f101098891))
// Same binary for many boards, attempts to detect CPU speed etc, currently failing detection but boots system roms anyway
	ROM_LOAD       ("fga002-3.1.rom",  0x800000, 0x10000, CRC (faa38972) SHA1 (651dfc2f9a865fc6adf49dad90f9e705f2889919) )
ROM_END

ROM_START (fccpu33) /* This is an original rom dump */
	ROM_REGION32_BE(0x900000, "roms", 0)
// Boots with Board ID set to: 0x36  (FGA002 BOOT and VMEPROM on terminal P4)
	ROM_LOAD16_BYTE("cpu33lo-ubr-v1.01.rom",  0x000001, 0x40000, CRC (49895fdf) SHA1 (733abd144c95225a2faf920490e31df2a27f8e03))
	ROM_LOAD16_BYTE("cpu33up-ubr-v1.01.rom",  0x000000, 0x40000, CRC (cfe75e94) SHA1 (d40e0635a48607be25f7c58c74b53b7e58fe735d))

// Same binary for many boards, attempts to detect CPU speed etc, currently failing detection but boots system roms anyway
	ROM_LOAD       ("fga002-3.1.rom",  0x800000, 0x10000, CRC (faa38972) SHA1 (651dfc2f9a865fc6adf49dad90f9e705f2889919) )
ROM_END

ROM_START (nodump)
	ROM_REGION32_BE(0x900000, "roms", 0)
	ROM_LOAD("rom.bin",  0x800000, 0x10000, NO_DUMP)
ROM_END

/* These needs reality check as they has 1Mb of RAM which is not a suitable size for later CPU_30:s */
#define rom_fccpu30x        rom_fccpu30
#define rom_fccpu30xa       rom_fccpu30
#define rom_fccpu30za       rom_fccpu30

#define rom_fccpu30zbe      rom_fccpu30
#define rom_fccpu30be8      rom_fccpu30
#define rom_fccpu30be16     rom_fccpu30

/* These needs dumps */
#define rom_fccpu30lite4    rom_nodump
#define rom_fccpu30lite8    rom_nodump

/* These are most likelly wrong, needs dump */
#define rom_fccpu30senr     rom_nodump
#define rom_fccpu30senr501  rom_nodump

/*
 * System ROM information
 *
 * FGA-002 Bootprom version 3.1 is released May 28, 1990, coprighted by FORCE Computers GmbH
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
void cpu30_state::fga_irq_callback(int)(01)
void fga002_device::check_interrupts()()
 */

/* Driver */
/*    YEAR  NAME            PARENT   COMPAT  MACHINE        INPUT  CLASS        INIT             COMPANY                 FULLNAME                  FLAGS */
COMP( 1988, fccpu30,        0,       0,      cpu30,         cpu30, cpu30_state, empty_init,      "Force Computers GmbH", "SYS68K/CPU-30",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1988, fccpu30x,       fccpu30, 0,      cpu30x,        cpu30, cpu30_state, init_cpu30x,     "Force Computers GmbH", "SYS68K/CPU-30X",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1988, fccpu30xa,      fccpu30, 0,      cpu30xa,       cpu30, cpu30_state, init_cpu30xa,    "Force Computers GmbH", "SYS68K/CPU-30XA",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1988, fccpu30za,      fccpu30, 0,      cpu30za,       cpu30, cpu30_state, init_cpu30za,    "Force Computers GmbH", "SYS68K/CPU-30ZA",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1996, fccpu30zbe,     fccpu30, 0,      cpu30zbe,      cpu30, cpu30_state, init_cpu30zbe,   "Force Computers GmbH", "SYS68K/CPU-30ZBE",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1996, fccpu30be8,     fccpu30, 0,      cpu30be8,      cpu30, cpu30_state, init_cpu30be8,   "Force Computers GmbH", "SYS68K/CPU-30BE/8",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1996, fccpu30be16,    fccpu30, 0,      cpu30be16,     cpu30, cpu30_state, init_cpu30be16,  "Force Computers GmbH", "SYS68K/CPU-30BE/16",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1996, fccpu30lite4,   fccpu30, 0,      cpu30lite4,    cpu30, cpu30_state, init_cpu30lite4, "Force Computers GmbH", "SYS68K/CPU-30Lite/4",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1996, fccpu30lite8,   fccpu30, 0,      cpu30lite8,    cpu30, cpu30_state, init_cpu30lite8, "Force Computers GmbH", "SYS68K/CPU-30Lite/8",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 199?, fccpu33,        fccpu30, 0,      cpu33,         cpu30, cpu30_state, init_cpu33,      "Force Computers GmbH", "SYS68K/CPU-33",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )

/* Below are not fully configured variants defaulting to generic cpu30 */

/* The following boards were manufactured for Ericsson to be used in their fixed network switches. They support hot swap and the Ericsson APNbus */
/* SYS68K/CPU-30SEN-R/32 assumed as generic until spec is found. 25 MHz 68030 based CPU board with DMAC, 32 MByte Shared RAM capacity and VMEPROM.
   4 MByte System Flash memory, SCSI via on-board EAGLE Controller FC68165 with DMA, 2 serial I/O ports, APNbus interface, VMEPROM firmware */
COMP( 1997, fccpu30senr,    0,       0,       cpu30,        cpu30, cpu30_state, empty_init, "Force Computers GmbH", "SYS68K/CPU-30SEN-R",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
/* SYS68K/CPU-30SEN-R-501/4 assumed as generic until spec is found. 25 MHz 68030 based CPU board with DMAC, 4 MByte Shared RAM capacity and VMEPROM.
   48V DC/DC onboard, metric backplane connectors, BYB501 PCB formfactor (TVJ807). 4 MByte System Flash memory, SCSI via onboard EAGLEController
   FC68165 with DMA, 2 serial I/O ports, APNbus interface, VMEPROM firmware*/
COMP( 1997, fccpu30senr501, 0,       0,       cpu30,        cpu30, cpu30_state, empty_init, "Force Computers GmbH", "SYS68K/CPU-30SEN-R-501", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )

/*CPU-33XB MC68030 25MHz CPU, 68882 FPC, 1MB, 2 SERIAL, RS-232, VME BOARD*/
//COMP( 1990, cpu33xb,        0,       0,       cpu30,        cpu30, cpu30_state, 0,          "Force Computers GmbH", "SYS68K/CPU-33XB",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
/*CPU-33B/4 MC68030 25MHz CPU, 68882 FPC, 1MB, 2 SERIAL, RS-232, VME BOARD*/
//COMP( 1990, cpu30b4,        0,       0,       cpu30,        cpu30, cpu30_state, 0,          "Force Computers GmbH", "SYS68K/CPU-33B/4",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
