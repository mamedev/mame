// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Motorola MVME series of CPU boards: MVME-162
 *
 *  16/05/2016, rebased 2019
 *
 *
 *       ||
 * ||    ||  MVME-162
 * ||||--||_____________________________________________________________
 * ||||--||                                                             |
 * ||    ||                                                           _ |__
 *       C|                                                          | |   |
 *       ||                                                          | |   |
 *       C|                                                          | |   |
 *       ||                                                          | |   |
 *       C|                                                          | |   |
 *       ||                                                          | |   |
 *       C|                                                          | |VME|
 *       ||                                                          | |   |
 *       ||                                                          | |P1 |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          |_|   |
 *       ||                                                            |___|
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |___
 *       ||                                                           _|   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |VME|
 *       ||                                                          | |   |
 *       ||                                                          | |P2 |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          |_|   |
 *       ||                                                            |___|
 * ||    ||                                                              +
 * ||||--||                                                              |
 * ||||--||--------------------------------------------------------------+
 * ||
 *
 * History of Motorola VME division (https://en.wikipedia.org/wiki/VMEbus)
 *---------------------------------
 * See mvme147.cpp
 *
 * Misc links about this board:
 * ----------------------------
 * http://bitsavers.informatik.uni-stuttgart.de/pdf/motorola/VME/MVME162/
 * http://www.m88k.com/mvme162.html
 *
 * Description(s)
 * -------------
 * MVME-162 has the following feature set
 *      - 25/33MHz MHzMC68040 or MC68LC040 Microprocessor
 *      - 1 or 4 MB of DRAM with parity protection on a mezzanine module, or 16 MB ECC DRAM on a mezzanine board
 *      - 128 KB of SRAM with battery backup, or 2 MB SRAM on a mezzanine board with battery backup
 *      - Four JEDEC standard 32-pin DIP PROM sockets
 *      - One Intel 28F008SA 1M x 8 Flash memory device with write protection.
 *      - Status LEDs for FAIL, RUN, SCON, and FUSES
 *      - 8K by 8 Non-Volatile RAM (NVRAM) and time of day (TOD) clock with battery backup
 *      - RESET and ABORT switches
 *      - Four 32-bit Tick Timers and Watchdog Timer (in the MCchip ASIC) for periodic interrupts
 *      - Two 32-bit Tick Timers and Watchdog Timer (in the VMEchip2 ASIC) for periodic interrupts
 *      - Eight software interrupts (for MVME162LX versions that have the VMEchip2)
 *      - Optional SCSI Bus interface with DMA
 *      - Four serial ports with EIA-232-D interface (serial port controllers are the Z85230s
 *      - Optional Ethernet transceiver interface with DMA Two IndustryPack interfaces
 * VMEbus interface
 *      - VMEbus system controller functions
 *      - VMEbus interface to local bus (A24/A32,
 *      - D8/D16/D32 (D8/D16/D32/D64 BLT) (BLT = Block Transfer)
 *      - Local bus to VMEbus interface (A16/A24/A32, D8/D16/D32)
 *      - VMEbus interrupter
 *      - VMEbus interrupt handler
 *      - Global CSR for interprocessor communications
 *      - DMA for fast local memory - VMEbus transfers (A16/A24/A32, D16/D32 (D16/D32/D64 BLT)
 *
 * Address Map
 * --------------------------------------------------------------------------
 *                          Decscription
 * --------------------------------------------------------------------------
 * 00000000-001FFFFF        Boot ROM until ROM0 bit is cleared
 * Programmable             DRAM on Parity Mezzanine D32 1-4MB
 * Programmable             DRAM on ECC Mezzanine D32 16MB
 * Programmable             On-board SRAM D32 128KB
 * Programmable             SRAM on Mezzanine D32 2MB
 * Programmable             VMEbus A32/A24 D32/D16
 * Programmable             IP_a Memory D32-D8 64KB-8MB
 * Programmable             IP_b Memory D32-D8 64KB-8MB
 * FF800000-FF9FFFFF        Flash/EPROM D32 2Mb
 * FFA00000-FFBFFFFF        EPROM/Flash D32 2Mb
 * FFC00000-FFDFFFFF        Not decoded
 * FFE00000-FFE1FFFF        On-board SRAM D32 128Kb
 * FFE80000-FFEFFFFF        Not decoded
 * ------------------------ Local I/O devices D8/D16/D32
 * FFF00000-FFF3FFFF        Reserved 256KB
 * FFF40000-FFF400FF        VMEchip2 (LCSR) D32 256B
 * FFF40100-FFF401FF        VMEchip2 (GCSR) D32-D8 256B
 * FFF40200-FFF40FFF        Reserved 3.5KB
 * FFF41000-FFF41FFF        Reserved 4KB
 * FFF42000-FFF41FFF        MCchip D32-D8 4KB
 * FFF43000-FFF430FF        MCECC #1 D8 256B
 * FFF43100-FFF431FF        MCECC #2 D8 256B
 * FFF43200-FFF43FFF        MCECC:s mirrored
 * FFF44000-FFF44FFF        Reserved
 * FFF45000-FFF45800        SCC #1 (Z85230) D8 2Kb
 * FFF45801-FFF45FFF        SCC #2 (Z85230) D8 2Kb
 * FFF46000-FFF46FFF        LAN (82596CA) D32 4Kb
 * FFF47000-FFF47FFF        SCSI (53C710) D32-D8 4Kb
 * FFF48000-FFF57FFF        Reserved
 * FFF58000-FFF587FF        IPIC IP_* D32-D16
 * FFF58800-FFF58FFF        Reserved
 * FFFBC000-FFFBC01F        IPIC Registers D32-D8
 * FFFBC800-FFFBFFFF        Reserved
 * FFFC0000-FFFC7FFF        MK48T08 (BBRAM, TOD Clock) D32-D8 32Kb
 * FFFC8000-FFFCBFFF        MK48T08 & Disable Flash writes D32-D8 16Kb
 * FFFC8000-FFFCBFFF        MK48T08 & Enable Flash writes D32-D8 16Kb
 * FFFD0000-FFFEFFFF        Reserved
 * FFFF0000-FFFFFFFF        VMEbux short I/O D16
 * --------------------------------------------------------------------------
 *
 *  TODO: (at a high level)
 *  - Add all SCC
 *  - Pass 162bug bootup tests
 *  - Add more divices as required by each board configuration (at least 30+ variants)
 *  - Write and add ASIC devices
 *  - Add local bus(es)
 *  - Add VME bus
 *  - Boot pSOS and VxWorks
 *  - Add variants of boards, preferably as runtime configurations
 *
 ****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68040.h"
#include "machine/z80scc.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
#include "machine/timekpr.h"

#define VERBOSE (0) // (LOG_GENERAL)
//#define LOG_OUTPUT_STREAM osd_printf_info
#include "logmacro.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

/*Serial Communications Interface
The MVME162LX uses two Zilog Z85230 serial port controllers to implement the four serial communications
interfaces. Each interface supports CTS, DCD, RTS, and DTR control signals; as well as the TXD and RXD
transmit/receive data signals. Because the serial clocks are omitted in the MVME162LX implementation,
serial communications are strictly asynchronous. The MVME162LX hardware supports serial baud rates of
110b/s to 38.4Kb/s. The Z85230 supplies an interrupt vector during interrupt acknowledge cycles.
The vector is modified based upon the interrupt source within the Z85230. Interrupt request levels are
programmed via the MCchip. The Z85230s are interfaced as DTE (data terminal equipment) with EIA-232-D
signal levels. The four serial ports are routed to four RJ45 telephone connectors on the MVME162LX front panel.*/


namespace {

/* This gives prompt at the RS232 terminal device (9600) */
#define BAUDGEN_CLOCK 10_MHz_XTAL // Not verified nor seen on the PCB:s
#define SCC_CLOCK (BAUDGEN_CLOCK)

class mvme162_state : public driver_device
{
public:
mvme162_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device (mconfig, type, tag)
		, m_maincpu (*this, "maincpu")
		, m_sccterm(*this, "scc")
	{
	}

	void mvme162(machine_config &confg);

private:
	uint32_t bootvect_r(offs_t offset);
	void bootvect_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	virtual void machine_start () override;
	virtual void machine_reset () override;
	void mvme162_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<scc85230_device> m_sccterm;
	//required_device<scc85230_device> m_sccterm2;

	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	uint32_t *m_sysrom = nullptr;
	uint32_t m_sysram[2]{};

	// PCC registers
	uint8_t m_genpurp_stat = 0;

	// VME chip registers
	uint8_t m_vc_cntl_conf = 0;
};

void mvme162_state::mvme162_mem(address_map &map)
{
	map.unmap_value_high();

	map(0x00000000, 0x00000007).ram().w(FUNC(mvme162_state::bootvect_w));       /* After first write we act as RAM */
	map(0x00000000, 0x00000007).rom().r(FUNC(mvme162_state::bootvect_r));       /* ROM mirror just during reset */

	map(0x00000008, 0x003fffff).ram(); /* 4 Mb RAM */

	map(0xff800000, 0xff9fffff).rom().region("roms", 0x800000); /* ROM/EEPROM bank 1 - 162bug/firmware */
	map(0xffa00000, 0xffbfffff).rom().region("roms", 0xa00000); /* ROM/EEPROM bank 2 - unpopulated/VxWorks/etc */

	map(0xffe00000, 0xffe1ffff).ram(); /* 128KB on board SRAM */

		/*  SGS-Thompson M48T18 RAM and clock chip, only 4088 bytes used,  and 8 bytes for the RTC, out of 8Kb though */
	map(0xfffc0000, 0xfffc7fff).rw("m48t18", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write));

	map(0xfff45000, 0xfff457ff).rw(m_sccterm, FUNC(scc85230_device::ab_dc_r), FUNC(scc85230_device::ab_dc_w)).umask32(0x00ff00ff); /* Port 1&2 - Dual serial port Z80-SCC */
}

/* Input ports */
static INPUT_PORTS_START (mvme162)
INPUT_PORTS_END

/* Start it up */
void mvme162_state::machine_start ()
{
	LOG("--->%s\n", FUNCNAME);

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (uint32_t*)(memregion ("roms")->base () + 0x800000);
	m_genpurp_stat = 0x02; /* Indicate power up reset */
	m_vc_cntl_conf = 0x01; /* We are the system controller */
}

void mvme162_state::machine_reset ()
{
	LOG("--->%s\n", FUNCNAME);

	/* Reset pointer to bootvector in ROM for bootvector handler bootvect_r */
	if (m_sysrom == &m_sysram[0]) /* Condition needed because memory map is not setup first time */
		m_sysrom = (uint32_t*)(memregion ("roms")->base () + 0x800000);
	m_genpurp_stat &= 0xfe; /* Clear parity error bit - not used by MAME at this point so just for the record */
}

/*
  Boot vector handler. Devices mapped at $FFF80000-$FFF9FFFF also appear at $00000000-$001FFFFF when the ROM0 bit
  in the MCchip EPROM control register is high (ROM0=1). ROM0 is set to 1 after each reset. The ROM0 bit must be
  cleared before other resources (DRAM or SRAM) can be mapped in this range ($00000000 - $001FFFFF).
*/
uint32_t mvme162_state::bootvect_r(offs_t offset)
{
	return m_sysrom[offset];
}

void mvme162_state::bootvect_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_sysram[offset % sizeof(m_sysram)] &= ~mem_mask;
	m_sysram[offset % sizeof(m_sysram)] |= (data & mem_mask);
	m_sysrom = &m_sysram[0]; // redirect all upcoming accesses to masking RAM until reset.
}

/*
 * Machine configuration
 */
void mvme162_state::mvme162(machine_config &config)
{
	M68040(config, m_maincpu, 25_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mvme162_state::mvme162_mem);

	M48T02(config, "m48t18", 0); /* t08 differs only in accepted voltage levels compared to t18 */

	/* Terminal Port config */
	SCC85230(config, m_sccterm, SCC_CLOCK);
	m_sccterm->out_txda_callback().set("rs232trm", FUNC(rs232_port_device::write_txd));
	m_sccterm->out_dtra_callback().set("rs232trm", FUNC(rs232_port_device::write_dtr));
	m_sccterm->out_rtsa_callback().set("rs232trm", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232trm(RS232_PORT(config, "rs232trm", default_rs232_devices, "terminal"));
	rs232trm.rxd_handler().set(m_sccterm, FUNC(scc85230_device::rxa_w));
	rs232trm.cts_handler().set(m_sccterm, FUNC(scc85230_device::ctsa_w));
}

/* ROM definitions */
ROM_START (mvme162)
	ROM_REGION32_BE(0xf00000, "roms", 0)
	ROM_DEFAULT_BIOS("162bug-v4.0")

	ROM_SYSTEM_BIOS(0, "162bug-v2.3", "MVME162 162bug v2.3")
	ROMX_LOAD("162bug_2.3.bin", 0x800000, 0x80000, CRC (301f52a8) SHA1 (ffc77561dce26a70020452baef76f4eb9dc14543), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "162bug-v4.0", "MVME162 162bug v4.0")
	ROMX_LOAD("162bug_4.0.bin", 0x800000, 0x80000, CRC (56728e5b) SHA1 (0b8b6725c21d8a9048d24857d6acd2b68a7f3ba0), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "probe", "MVME162 pSOS pROBE+ boot loader")
	ROMX_LOAD("162probe+_3.1.0.bin", 0x800000, 0x80000, CRC (1d050793) SHA1 (d060fbbf548b2559c0d251fae5a2eb87b0132f0b), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "vxworks", "MVME162 WindRiver VxWorks boot loader")
	ROMX_LOAD("162bug_2.3.bin", 0x800000, 0x80000, CRC (301f52a8) SHA1 (ffc77561dce26a70020452baef76f4eb9dc14543), ROM_BIOS(3))
	ROMX_LOAD("vxworks_5.1.1_162lx-223.bin", 0xa00000, 0x20000, CRC (b40b39ac) SHA1 (fbc7f7e05ff276fe4570daeadcc5c08fc11f1a2b), ROM_BIOS(3))
ROM_END

} // anonymous namespace


/* Driver */
//    YEAR  NAME     PARENT  COMPAT   MACHINE  INPUT     CLASS          INIT        COMPANY       FULLNAME    FLAGS
COMP (1993, mvme162, 0,      0,       mvme162, mvme162,  mvme162_state, empty_init, "Motorola",   "MVME-162", MACHINE_NOT_WORKING)
