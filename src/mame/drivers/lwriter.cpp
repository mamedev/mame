// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/******************************************************************************

    Apple LaserWriter II NT driver

    TODO: 
	- Figure out what VIA pins is connected to switch on front that selects LocalTalk
    - Let the board identify itself to a emulated mac driver so it displays the printer icon on the desktop
    - Everything else

******************************************************************************/
/*
 * Hardware: 68000@11.16 MHz
             8530 SCC
			 6523 TPI or 6522 VIA on newer pcb:s
			 2MB DRAM
			 2KB SRAM
			 custom 335-0022 EEPROM
			 1MB ROM

   +------------------------------------------------------------------------------------------------------------------------+=====+
   |      1           2            3           4            5          6          7          8        9        10     11    |     #
   |    +------+    +------+   +---------+                                                +-------+  +------+         +-+   |     #
   |A   |511000|    | F257 |   |335-0022 |                                                | 0296  |  |22.3210         | | J2|     #
   |    +------+    +------+   |EEPROM   |                     +-------+                  +-------+  |XTAL  |         | |   |     #
   |B   |511000|    | RP2B |   +---------+                     |       |                             +------+         | |   |     #
   |    +------+    +------+                                   | 68000 |                  +-------+  +-------+        +-+   |     #
   |C   |511000|    | F257 |                                   |       |                  | 0558  |  | F175  |        +-+   |     #
   |    +------+    +------+   +---------+                     |       |                  +-------+  +-------+        | |   |     #
   |D   |511000|    +------+   |Am9128-10|                     |       |        +-------+ +-------+  +-------+        | |   |     #
   |    +------+    | F257 |   |SRAM     |                     |       |        | 0559  | | 0557  |  | LS393 |        | |   |     #
   |E   |511000|    +------+   +---------+                     |       |        +-------+ +-------+  +-------+        +-+   |  F  #
   |    +------+    +------+   +---------+                     |       |                                      +------+      |     #
   |F   |511000|    | F138 |   |Am9128-10|                     |       |                +-----------------+   |26LS32|    J3|  R  #
   |    +------+    +------+   |SRAM     |                     |       |                | Z8530B1C        |   +------+      |     #
   |G   |511000|               +--------++                     |       |                | SCC             |   |26LS32|      |  O  #
   |    +------+    +------+   | F244   |                      |       |     +----+     +-----------------+   +------+      |     #
   |H   |511000|    | RP2H |   +--------+                      +-------+     |7705|                           +------+      |  N  #
   |    +------+    +------+   | F244   |                                    +----+     +-----------------+   |26LS30|      |     #
   |J   |511000|    | 0259 |   +--------+-+----------+                   o------o       | 338-6523        |   +------+      |  T  #
   |    +------+    +------+   | TC531000 | TC531000 |                   | CONN |       | TPI             |   |26LS30|      |     #
   |K   |511000|    +------+   |  ROM H3  |  ROM L3  |          +------+ |   == |       +-----------------+   +------+      |     #
   |    +------+    | RP2L |   +----------+----------+          | F02  | |   == |             +-------+                     |     #
   |L   |511000|    +------+   +----------+----------+          +------+ |   == |             | LS14  |                     |     #
   |    +------+    | 0259 |   | TC531000 | TC531000 |  +------++------+ |   == |             +-------+                     |     #
   |M   |511000|    +------+   |  ROM H2  |  ROM L2  |  |67L401|| LS166| |   == |             | LS14  |                     |     #
   |    +------+    +------+   +----------+----------+  +------++------+ |   == |             +-------+ +------+            |     #
   |N   |511000|    | F245 |   +----------+----------+  +------++------+ |   == |                       | 0440A|            |     #
   |    +------+    +------+   | TC531000 | TC531000 |  |67L401|| LS00 | |   == |                       +------+            |     #
   |P   |511000|    +------+   |  ROM H1  |  ROM L1  |  +------++------+ o------o      +------+                             |     #
   |    +------+    | F245 |   +----------+----------+         +-------+               |TL497 |                             |     #
   |R   |511000|    +------+   +----------+----------+         | LS273 |               +------+                             |     #
   |    +------+    +------+   | TC531000 | TC531000 |         +-------+                                                    |     #
   |S   |511000|    | RP2S |   |  ROM H0  |  ROM L0  |         | LS05 |                                                     |     #
   |    +------+    +------+   +----------+----------+         +------+  (c)1987                                          J4|     #
   |     DRAM                       LASERWRITER II NT                    Apple Computer               640-4105              |     #
   +------------------------------------------------------------------------------------------------------------------------+=====+
 */

#define TPI 0  //The TPI is used on the original M6009 board but the first dump is from a newer that uses a VIA 

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "bus/rs232/rs232.h"
#include "machine/z80scc.h"

#if TPI
#include "machine/6525tpi.h"
#else
#include "machine/6522via.h"
#endif 

class lwriter_state : public driver_device
{
public:
	lwriter_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_scc(*this, "scc")
		, m_via(*this, "via")
	{ }
	DECLARE_READ16_MEMBER (bootvect_r);
	DECLARE_WRITE16_MEMBER (bootvect_w);
	virtual void machine_start () override;
	virtual void machine_reset () override;
private:
	required_device<cpu_device> m_maincpu;
	required_device<scc8530_device> m_scc;

#if TPI
#define tpi6523_device tpi6525_device/* TODO: define a type for the TPI6523 of its own in the device header file */
	required_device<tpi6523_device> m_tpi;
#else
	required_device<via6522_device> m_via;
#endif

	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	uint16_t  *m_sysrom;
	uint16_t  m_sysram[2];
};

static ADDRESS_MAP_START (maincpu_map, AS_PROGRAM, 16, lwriter_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x00000007) AM_ROM AM_READ(bootvect_r) /* ROM mirror just during reset */
	AM_RANGE(0x00000000, 0x00000007) AM_RAM AM_WRITE(bootvect_w) /* After first write we act as RAM */
	AM_RANGE(0x00000008, 0x001fffff) AM_RAM /* 2 Mb DRAM */
	AM_RANGE(0x00200000, 0x003fffff) AM_ROM AM_REGION("roms", 0) 

	AM_RANGE(0x00c00000, 0x00c00001) AM_DEVWRITE8("scc", scc8530_device, ca_w, 0x00ff)
	AM_RANGE(0x00c00004, 0x00c00005) AM_DEVWRITE8("scc", scc8530_device, da_w, 0x00ff)
	AM_RANGE(0x00a00000, 0x00a00001) AM_DEVREAD8 ("scc", scc8530_device, ca_r, 0xff00)
	AM_RANGE(0x00a00004, 0x00a00005) AM_DEVREAD8 ("scc", scc8530_device, da_r, 0xff00)

	AM_RANGE(0x00c00002, 0x00c00003) AM_DEVWRITE8("scc", scc8530_device, cb_w, 0x00ff)
	AM_RANGE(0x00c00006, 0x00c00007) AM_DEVWRITE8("scc", scc8530_device, db_w, 0x00ff)
	AM_RANGE(0x00a00002, 0x00a00003) AM_DEVREAD8 ("scc", scc8530_device, cb_r, 0xff00)
	AM_RANGE(0x00a00006, 0x00a00007) AM_DEVREAD8 ("scc", scc8530_device, db_r, 0xff00)

#if TPI
	AM_RANGE(0x00e00010, 0x00e0001f) AM_DEVREADWRITE8 ("tpi", tpi6523_device, read, write, 0x00ff) // Used on older boards, needs proper mapping
#else
	AM_RANGE(0x00e00000, 0x00e0001f) AM_DEVREADWRITE8 ("via", via6522_device, read, write, 0x00ff)
#endif
ADDRESS_MAP_END

static INPUT_PORTS_START( lwriter )
INPUT_PORTS_END

/* Start it up */
void lwriter_state::machine_start()
{
	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (uint16_t*)(memregion ("roms")->base ());
}

void lwriter_state::machine_reset ()
{
	/* Reset pointer to bootvector in ROM for bootvector handler bootvect_r */
	if (m_sysrom == &m_sysram[0]) /* Condition needed because memory map is not setup first time */
		m_sysrom = (uint16_t*)(memregion ("roms")->base ());
}

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0xff800000 to 0x0 at reset*/
READ16_MEMBER (lwriter_state::bootvect_r){
	return m_sysrom[offset];
}

WRITE16_MEMBER (lwriter_state::bootvect_w){
	m_sysram[offset % sizeof(m_sysram)] &= ~mem_mask;
	m_sysram[offset % sizeof(m_sysram)] |= (data & mem_mask);
	m_sysrom = &m_sysram[0]; // redirect all upcomming accesses to masking RAM until reset.
}

#define CPU_CLK (XTAL_22_3210MHz / 2) // Based on pictures form here: http://picclick.co.uk/Apple-Postscript-LaserWriter-IINT-Printer-640-4105-M6009-Mainboard-282160713108.html#&gid=1&pid=7
#define RXC_CLK ((CPU_CLK - (87 * 16 * 70)) / 3) // Tuned to get 9600 baud according to manual, needs rework based on real hardware

static MACHINE_CONFIG_START( lwriter, lwriter_state )
	MCFG_CPU_ADD("maincpu", M68000, CPU_CLK)
	MCFG_CPU_PROGRAM_MAP(maincpu_map)
	MCFG_SCC8530_ADD("scc", CPU_CLK, RXC_CLK, 0, RXC_CLK, 0)
	/* Port A */
	MCFG_Z80SCC_OUT_TXDA_CB(DEVWRITELINE("rs232a", rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_DTRA_CB(DEVWRITELINE("rs232a", rs232_port_device, write_dtr))
	MCFG_Z80SCC_OUT_RTSA_CB(DEVWRITELINE("rs232a", rs232_port_device, write_rts))
	/* Port B */
	MCFG_Z80SCC_OUT_TXDB_CB(DEVWRITELINE("rs232b", rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_DTRB_CB(DEVWRITELINE("rs232b", rs232_port_device, write_dtr))
	MCFG_Z80SCC_OUT_RTSB_CB(DEVWRITELINE("rs232b", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD ("rs232a", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("scc", scc8530_device, rxa_w))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("scc", scc8530_device, ctsa_w))

	MCFG_RS232_PORT_ADD ("rs232b", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("scc", scc8530_device, rxb_w))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("scc", scc8530_device, ctsb_w))

#if TPI
	MCFG_DEVICE_ADD("tpi", TPI6525, 0)
#else
	MCFG_DEVICE_ADD("via", VIA6522, 0)
#endif
MACHINE_CONFIG_END

/* SCC init sequence
 * :scc B Reg 09 <- c0 - Master Interrupt Control - Device reset
 * -
 * :scc A Reg 0f <- 00 - External/Status Control Bits - Disable all 
 * :scc B Reg 05 <- 02 - Tx setup: 5 bits, Tx disable, RTS:1 DTR:0  
 * :scc B Reg 05 <- 00 - Tx setup: 5 bits, Tx disable, RTS:0 DTR:0  
 * -
 * :scc A Reg 09 <- c0 - Master Interrupt Control - Device reset
 *
 * -
 * :scc A Reg 0f <- 00 - External/Status Control Bits - Disable all 
 * :scc A Reg 04 <- 4c - Setting up Asynchrounous mode: 2 Stop bits, No parity, 16x clock
 * :scc A Reg 0b <- 50 - Clock Mode Control - TTL clk on RTxC, Rx and Tx clks from BRG, TRxC is input
 * :scc A Reg 0e <- 00 - Misc Control Bits - BRG clk is RTxC, BRG is disabled
 * :scc A Reg 0c <- 0a - Low byte of baudrate generator constant
 * :scc A Reg 0d <- 00 - Hi byte of baudrate generator constant
 * :scc A Reg 0e <- 01 - BRG enabled with external clk from RTxC
 * :scc A Reg 0a <- 00 - Synchronous parameters, all turned off
 * :scc A Reg 03 <- c1 - Rx setup: 8 bits, Rx enabled
 * :scc A Reg 05 <- 6a - Tx setup: 8 bits, Tx enable, RTS:1 DTR:0 
 * -
 * :scc A Reg 01 <- 00 - Rx interrupt disabled
 * :scc A Reg 01 <- 30 - Wait/Ready on receive, Rx int an all characters, parity affect vector
 * :scc A Reg 00 <- 30 - Error Reset command
 * -
 * :scc A Reg 01 <- 01 - External interrupt enabled, Rx ints disabled
 * :scc A Reg 00 <- 30 - Error Reset command
 * :scc A Reg 00 <- 30 - Error Reset command
 * - last three loops
*/ 

ROM_START(lwriter)
	ROM_REGION16_BE (0x1000000, "roms", 0)

	ROM_LOAD16_BYTE ("342-0545.l0", 0x000001, 0x20000, CRC (6431742d) SHA1 (040bd5b84b49b86f2b0fe9ece378bbc7a10a94ec))
	ROM_LOAD16_BYTE ("342-0546.h0", 0x000000, 0x20000, CRC (c592bfb7) SHA1 (b595ae225238f7fabd1566a3133ea6154e082e2d))
	ROM_LOAD16_BYTE ("342-0547.l1", 0x040001, 0x20000, CRC (205a5ea8) SHA1 (205fefbb5c67a07d57cb6184c69648321a34a8fe))
	ROM_LOAD16_BYTE ("342-0548.h1", 0x040000, 0x20000, CRC (f616e1c3) SHA1 (b9e2cd4d07990b2d1936be97b6e89ef21f06b462))
	ROM_LOAD16_BYTE ("342-0549.l2", 0x080001, 0x20000, CRC (0b0b051a) SHA1 (64a80085001570c3f99d9865031715bf49bd7698))
	ROM_LOAD16_BYTE ("342-0550.h2", 0x080000, 0x20000, CRC (82adcf85) SHA1 (e2ab728afdae802c0c67fc25c9ba278b9cb04e31))
	ROM_LOAD16_BYTE ("342-0551.l3", 0x0c0001, 0x20000, CRC (176b3346) SHA1 (eb8dfc7e44f2bc884097e51a47e2f10ee091c9e9))
	ROM_LOAD16_BYTE ("342-0552.h3", 0x0c0000, 0x20000, CRC (69b175c6) SHA1 (a84c82be1ec7e373bb097ee74b941920a3b091aa))
ROM_END

/*    YEAR  NAME        PARENT    COMPAT  MACHINE    INPUT      INIT,             COMPANY,                FULLNAME,                   FLAGS */
CONS( 1988, lwriter,    0,        0,      lwriter,   lwriter,   driver_device, 0, "Apple",                "Apple Laser Writer II NT", MACHINE_IS_SKELETON)
