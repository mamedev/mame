// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Heurikon HK68/V10 6U SBC driver, initially derived from hk68v10.c
 *
 *  21/08/2015
 *
 * I baught this board from http://www.retrotechnology.com without documentation.
 * It has a Motorola 68010 CPU @ 10MHz and two 2764 EPROMS with HBUG firmware
 * The board is very populated and suitable to run a real server OS supported by
 * FPU,MMU and DMA controller chips. The firmware supports SCSI, Centronics/FPI
 * and a serial port. I have not found so much on the FPI interface yet and it is
 * possibly Heurikon specific. There is also a Heurikon Multibus board called
 * HK68/M10 for which I might add support from specs, however would need a ROM
 * to verify.
 *
 *       ||
 * ||    ||
 * ||||--||
 * ||||--||
 * ||    ||____________________________________________________________   ___
 *       ||     |     |        | 12 PALs 68/V-...    |   |   |   |   | |_|   |
 *       ||     |     |  2 x   |                     |74 |74 |74 |74 | | |   |
 *       ||     |     |  74273 | VS4-01     VM4-00   | 244 573 645 645 | |   |
 *   \==|||     |SCSI |        | VS3-00     VM3-03   |   |   |   |   | | |   |
 *      |||     |     |        | VS2-01     VM2-02   |___|___|___|___| | |   |
 *       ||     |NCR  |        | VS1-00     VM1-01   |   |   |   |     | |   |
 *       ||     | 8530|  2 x   | VBX-00     VDB-00   |74 |74 |74 |     | |VME|
 *       ||     |     |  74245 | VI1-00     VI2-00   | 244 641 245     | |   |
 *       ||     |     |        | (90 deg ccw)        |   |   |   |     | |P1 |
 * SCSI  ||     |_____|        |_____________________|___|___|___|     | |   |
 * port  ||     |     |  _________  _________________________________  | |   |
 *       ||     |     | |         ||                 |74 |PAL |      | | |   |
 *       ||     |     | |         ||   5 x           | 244 MP2| 2 x  | | |   |
 *       ||     |CIO  | | MC68881 ||   74373         |   | -00| 74646| | |   |
 *       ||     |paral| | FPU     ||                 |   |    |      | | |   |
 *       ||     |Z8536| |_________||_________________|_ _|    |      | |_|   |
 *      |||     |     | |         |          |         |  ------------   |___|
 *   /==|||     |     | |         |          |         |  | 3 PALs   |   |
 * FSM   ||     |     | | MC68010 | MC68451  | MC68450 |  |  MP1-00  |   |
 * 1234  ||     |     | | CPU     | MMU      | DMA     |  |  MX2-00  |   |
 * LEDs  ||     |-----| |_________|__________|_________|__|  MX1-01  |---|
 *       ||     |     |  ____   ____   ---|               |__________|74 |
 *   \==|||     |     | |    | |    |  |74|     4 x       | 3 PALs   |145|
 *      |||     |SCC  | |    | |    |  |804     74245     |  CS3-00  |___|
 * FPI   ||     |seria| |U12 | |U23 |  |  |               |  CS2-00  |   |
 * paralell     |Z8530A |HBUG| |HBUG|  |  |               |  CS1-00  |   |___
 * port  ||     |     | |    | |    |  ---_---------------|__________|  _|   |
 *       ||     |     | |    | |    | |MTTLDL|MDLDM| 2 x |___________  | |   |
 *      |||     |     | |____|_|____|_|____40|TTL75|74280|          |  | |   |
 *   /==|||     |     ||   |19. |     |    |  |  | |_____|          |  | |   |
 *   \==|||     |_____||75 |6608|16   |20  |74|74| ______|  9 x 4   |  | |   |
 *      |||  +--------+|173| MHz| MHz | MHz| 74 08|   |  | RAM chips|  | |VME|
 *       ||  |        ||___|____|_____|____|__|__||74 74 | on SIL   |  | |   |
 *       ||  | 2 x    ||                   |  |   |259|257 boards   |  | |P2 |
 *       ||  | SIL    || 12 PALs 90 deg ccw|74|   -______-          |  | |   |
 *       ||  | boards ||                   |240   |   |  |          |  | |   |
 *       ||  | with 2 || BER-00   FPI-01   |  |   |74 74 |          |  | |   |
 * Serial||  | DIP16  || AC4-00   ARB-01   |__|___|138|257          |  | |   |
 * port  ||  | surf   || AC3-00   INT-00          |______|          |  | |   |
 *       ||  | mounted|| AC2-01   IAK-00          |   |  |          |  | |   |
 *       ||  | chips  || AC1-01   PAS-01          |74 |74|          |  | |   |
 *       ||  | each   || RFS-01                   |590|257          |  | |   |
 *      |||  |        || RCT-01                   |   |  |          |  |_|   |
 *   /==|||  +--------+|__________________________|___|__|__________|    |___|
 * ||    ||------------------------------------------------------------+-+
 * ||||--||
 * ||||--||
 * ||
 *
 * History of Heurikon
 *---------------------
 * The company was founded 1972 as cellar company. Heurikon was aquired
 * 1989 by Computer Products, 1990 by Artesyn and finally in 2005 by Emerson
 * Electric who consilidated it fully by 2009 and closed the office.
 *
 * Misc links about Heurikon and this board:
 * http://www.heurikon.com/
 * http://www.nytimes.com/1992/01/07/business/company-news-briefs.html
 * http://bitsavers.informatik.uni-stuttgart.de/pdf/heurikon/brochures/HK68_V10_Brochure.pdf
 * http://bitsavers.informatik.uni-stuttgart.de/pdf/heurikon/Heurikon_UNIX_System_V_and_V.2_Reference_Guide_Apr87.pdf
 * From http://www.iri.tudelft.nl/~sfwww/manuals/OS9/PDF/9OM_30.pdf:
 * "In most cases, you will only need to change the device base address.  Some hardware implementations of the MC68451
 *  (specifically the Heurikon M10/V10 CPU's) use the DMA portion of the Address Space Table (AST) instead of the MPU
 *  section which is normally used.  You should change the offsets for the AST registers to match your hardware.   The
 *  ssmdefs.a file has conditional directives to accommodate either the standard or Heurikon style implementations."
 *
 * Address Map from the UNIX Ref Guide
 * --------------------------------------------------------------------------
 * Address Range     Memory Space (physical)   Notes
 * --------------------------------------------------------------------------
 * 0xffffff                                    (top of memory)
 * 0xffc200 0xffc220 Sky FFP                   68881 FPU
 * 0xff8000   ...    MCT                       Reel-to-reel tape (V10) VME
 * 0xff0000 0xff0100 Tapemaster(00B0,00B1)     Reel-to-reel tape (M10) Multibus
 *                   Ethernet(0010,0011)
 * 0xff0000                                    (base of bus I/O)
 *
 * 0xfea000          SCC8530 device
 * 0xfe9000          offset 6 seems also to be 2 access registers, ie register offset + data pairs
 * 0xfe8000          offset E reading single byte
 * 0xfe4000          front LED:s? or move ROM from 0x0 to 0xfc0000...
 * 0xfe2000          cleared
 * 0xfe0000          HK68 On-card I/O          MMU, DMAC, SCC, SCSI
 * 0xfc0000          HK68 ROM (128K)           Hbug monitor
 * 0xf00000          (reserved)
 *                                             (shared memory and logical area
 *                                              for shared text.)
 * 0x800000          (Twilight Zone)           (top of /dev/mem)
 * 0x7c0000          (reserved)
 * 0x780000          8 Chnl serial Expn(1-4)   (CDC)
 *                   HK68 (1-15)               VRTX
 *                   MLZ-93 (1-4)              CP/M Shell
 * 0x700000
 *                   (open)
 * 0x*00000                                    (maxmeml)
 *                   RAM (bus)                 (optional)
 * 0x200000                                    (top of 2 meg RAM)
 *                   RAM (bus)                 (optional)
 * 0x100000                                    (top of 1 meg RAM)
 *                   User RAM
 * 0x001000          UNIX Kernel
 *                   Exception Vectors
 * 0x000000                                    (bottom of memory)
 * --------------------------------------------------------------------------
 *
 * Interrupt sources M10/V10
 * ----------------------------------------------------------
 * Description                  Device  Lvl  IRQ    VME board
 *                           /Board      Vector  Address
 * ----------------------------------------------------------
 * On board Sources                     M10/V10
 *
 * Off board Sources (other VME boards)
 *  EXOS Ethernet                        5   4
 *  TapeMaster Reel-to-Reel Tape         3
 *  MCT Reel-to-Reel Tape                    5
 *  CDC MB1031 Serial Expansion          2   2
 *  IT 2190 SMD Interface                1   3
 *  IT 3200 SMD Interface                    3
 * ----------------------------------------------------------
 *
 * DMAC Channel Assignments
 * ----------------------------------------------------------
 * Channel         M10            V10
 * ----------------------------------------------------------
 *   0             SCSI           SCSI
 *   1             Streamer (P3)  n/a
 *   2             n/a            n/a
 *   3             SBX-FDIO       n/a
 * ----------------------------------------------------------
 *
 *  TODO:
 *  - Dump the ROMs (DONE)
 *  - Setup a working address map (DONE)
 *  - Fix terminal for HBUG (DONE)
 *  - Add VME bus driver
 *  - Add DMA/MMU devices
 *  - Add CIO port
 *  - ADD SCSI controller device
 *  - dump PALs and describe descrete logic
 *  - Setup BAUD generation correctly, (eg find that x32 divider)
 *     it is a multilayer PCB so very hard to trace.
 *  - Add LED:s
 *  - Add Jumpers and strap areas
 *  - Find and Boot Heurikon Unix from a SCSI device
 *  - Support that optional 68881 FPU
 *
 ****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/z80scc.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#if VERBOSE == 2
#define logerror printf
#endif

#define BAUDGEN_CLOCK XTAL_19_6608MHz /* Raltron */
/*
 */
#define SCC_CLOCK (BAUDGEN_CLOCK / 128) /* This gives prompt at the RS232 terminal device (9600) */
//#define SCC_CLOCK (BAUDGEN_CLOCK / 4) /* This is correct giving 4.9152MHz as documentation says */
class hk68v10_state : public driver_device
{
public:
hk68v10_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device (mconfig, type, tag),
		m_maincpu (*this, "maincpu")
		,m_sccterm(*this, "scc")
//        ,m_cart(*this, "exp_rom1")
{
}

DECLARE_READ16_MEMBER (bootvect_r);
DECLARE_WRITE16_MEMBER (bootvect_w);
DECLARE_READ16_MEMBER (vme_a24_r);
DECLARE_WRITE16_MEMBER (vme_a24_w);
DECLARE_READ16_MEMBER (vme_a16_r);
DECLARE_WRITE16_MEMBER (vme_a16_w);
virtual void machine_start ();
virtual void machine_reset ();
DECLARE_WRITE_LINE_MEMBER (write_sccterm_clock);

protected:

private:
required_device<cpu_device> m_maincpu;
required_device<scc8530_device> m_sccterm;

// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	UINT16  *m_sysrom;
	UINT16  m_sysram[4];
};

static ADDRESS_MAP_START (hk68v10_mem, AS_PROGRAM, 16, hk68v10_state)
ADDRESS_MAP_UNMAP_HIGH
AM_RANGE (0x000000, 0x000007) AM_ROM AM_READ  (bootvect_r)       /* ROM mirror just durin reset */
AM_RANGE (0x000000, 0x000007) AM_RAM AM_WRITE (bootvect_w)       /* After first write we act as RAM */
AM_RANGE (0x000008, 0x1fffff) AM_RAM /* 2 Mb RAM */
AM_RANGE (0xFC0000, 0xFC3fff) AM_ROM /* System EPROM Area 16Kb HBUG */
AM_RANGE (0xFC4000, 0xFDffff) AM_ROM /* System EPROM Area an additional 112Kb for System ROM */
AM_RANGE (0xFE9000, 0xFE9009) AM_RAM //AM_DEVREADWRITE8("scc", scc8530_device, ba_cd_r, ba_cd_w, 0xffff) /* Z80-PIO? */
AM_RANGE (0xFEA000, 0xFEA001) AM_DEVREADWRITE8("scc", scc8530_device, ca_r, ca_w, 0xff00) /* Dual serial port Z80-SCC */
AM_RANGE (0xFEA002, 0xFEA003) AM_DEVREADWRITE8("scc", scc8530_device, cb_r, cb_w, 0xff00) /* Dual serial port Z80-SCC */
AM_RANGE (0xFEA004, 0xFEA005) AM_DEVREADWRITE8("scc", scc8530_device, da_r, da_w, 0xff00) /* Dual serial port Z80-SCC */
AM_RANGE (0xFEA006, 0xFEA007) AM_DEVREADWRITE8("scc", scc8530_device, db_r, db_w, 0xff00) /* Dual serial port Z80-SCC */
//AM_RANGE(0x100000, 0xfeffff)  AM_READWRITE(vme_a24_r, vme_a24_w) /* VMEbus Rev B addresses (24 bits) - not verified */
//AM_RANGE(0xff0000, 0xffffff)  AM_READWRITE(vme_a16_r, vme_a16_w) /* VMEbus Rev B addresses (16 bits) - not verified */
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START (hk68v10)
INPUT_PORTS_END

/* Start it up */
void hk68v10_state::machine_start ()
{
		LOG (("%" I64FMT "d %s\n", m_maincpu->total_cycles(), __func__));

		/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (UINT16*)(memregion ("maincpu")->base () + 0x0fc0000);
}

/* Support CPU resets

  TODO: Investigate why the user need two 'softreset' commands for the below to work. Eg F3 + F11 + F3 + F11
        If only one 'softreset' is given the reset PC gets the RAM content, not the intended ROM vector.
        Race conditions? Wrong call order in memory system? Debugger prefetch accesses? Better way to to this?
*/
void hk68v10_state::machine_reset ()
{
		LOG (("%" I64FMT "d %s\n", m_maincpu->total_cycles(), __func__));

		/* Reset pointer to bootvector in ROM for bootvector handler bootvect_r */
		if (m_sysrom == &m_sysram[0]) /* Condition needed because memory map is not setup first time */
			m_sysrom = (UINT16*)(memregion ("maincpu")->base () + 0x0fc0000);
}

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0xfc0000 to 0x0 at reset*/
/*
  Right after HBUG reset the bootvector is masked by RAM:
  FC001C: move.w  #$700, $fe4000.l
  FC0024: move.l  #$0, $0.l # <- zeroing the reset vector
  FC002E: move.l  #$0, $4.l # There is for sure some hardware mapping going in here
*/
READ16_MEMBER (hk68v10_state::bootvect_r){
		//LOG (("bootvect_r %s\n", m_sysrom != &m_sysram[0] ? "as reset" : "as swapped"));
		return m_sysrom[offset];
}

WRITE16_MEMBER (hk68v10_state::bootvect_w){
		LOG (("bootvect_w offset %08x, mask %08x, data %04x\n", offset, mem_mask, data));
		m_sysram[offset % sizeof(m_sysram)] &= ~mem_mask;
		m_sysram[offset % sizeof(m_sysram)] |= (data & mem_mask);
		m_sysrom = &m_sysram[0]; // redirect all upcomming accesses to masking RAM until reset.
}

#if 0
/* Dummy VME access methods until the VME bus device is ready for use */
READ16_MEMBER (hk68v10_state::vme_a24_r){
		LOG (("vme_a24_r\n"));
		return (UINT16) 0;
}

WRITE16_MEMBER (hk68v10_state::vme_a24_w){
		LOG (("vme_a24_w\n"));
}

READ16_MEMBER (hk68v10_state::vme_a16_r){
		LOG (("vme_16_r\n"));
		return (UINT16) 0;
}

WRITE16_MEMBER (hk68v10_state::vme_a16_w){
		LOG (("vme_a16_w\n"));
}
#endif

/*
 * Serial port clock source depends on the CPU clock and divider settings
 * HBUG has a control byte that needs to be patched accordingly:
 *
 * CPU clock       SCC clock      Baud rate   offset 0x0b value
 *  10 Mhz         4.9152MHz      9600          15
 *  10 Mhz         4.9152MHz     19200          55
 *  12 Mhz         4.9152MHz      9600          16
 *  12 Mhz         4.9152MHz     19200          56
 *
 * Configuration word detail:
 * D15-D8 reserved ( = 0 )
 * D7(MSB) = 0
 * D6 = 0 for console default 9600 baud
 * D6 = 1 for console default 19,200 baud
 * D5,D4,D3,D2 = 0101 for 4.9152 Mhz SCC clock
 * D1,DO
 * 01 for 10 Mhz MPU clock
 * D1,DO = 10 for 12 Mhz MPU clock
 *
 * Original HBUG configuration word: 0x003D = 0000 0000 0011 1101
 *
 */
WRITE_LINE_MEMBER (hk68v10_state::write_sccterm_clock){
		m_sccterm->txca_w (state);
		m_sccterm->rxca_w (state);
}

/*
 * Machine configuration
 */
static MACHINE_CONFIG_START (hk68v10, hk68v10_state)
/* basic machine hardware */
MCFG_CPU_ADD ("maincpu", M68010, XTAL_10MHz)
MCFG_CPU_PROGRAM_MAP (hk68v10_mem)

/* Terminal Port config */
MCFG_SCC8530_ADD("scc", XTAL_4MHz, 0, 0, 0, 0 )
MCFG_Z80SCC_OUT_TXDA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_txd))
MCFG_Z80SCC_OUT_DTRA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_dtr))
MCFG_Z80SCC_OUT_RTSA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_rts))

MCFG_RS232_PORT_ADD ("rs232trm", default_rs232_devices, "terminal")
MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("scc", scc8530_device, rxa_w))
MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("scc", scc8530_device, ctsa_w))

MCFG_DEVICE_ADD ("sccterm_clock", CLOCK, SCC_CLOCK)
MCFG_CLOCK_SIGNAL_HANDLER (WRITELINE (hk68v10_state, write_sccterm_clock))

MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (hk68v10)
ROM_REGION (0x1000000, "maincpu", 0)

ROM_LOAD16_BYTE ("hk68kv10U23.bin", 0xFC0001, 0x2000, CRC (632aa026) SHA1 (f2b1ed0cc38dfbeb1602c013e00757015400720d))
ROM_LOAD16_BYTE ("hk68kv10U12.bin", 0xFC0000, 0x2000, CRC (f2d688e9) SHA1 (e68699965645f0ce53de47625163c3eb02c8b727))
/*
 * System ROM information
 *
 * The ROMs contains HBUG v1.8, known commands from different sources:
 *
 *  'uc'       Print HK68 Configuration
 *  'um'       Perform RAM test
 *  'dm adrs'  Display Memory
 *  'sb adrs'  Substitute Byte at adrs
 *  'c adrs'   Call Routine at adrs
 *  'bw'       Boot from Winchester
 *  'bf'       Boot from floppy (MIO, SBX-FDIO)
 *  'bsf'      Boot from floppy (SCSI)
 *
 */
ROM_END

/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT COMPANY                  FULLNAME          FLAGS */
COMP (1985, hk68v10,      0,      0,       hk68v10,        hk68v10, driver_device, 0,   "Heurikon Corporation",   "HK68/V10", MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
