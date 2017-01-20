// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Force SYS68K CPU-20 VME SBC drivers
 *
 *  24/12/2016
 *
 * Thanks to Al Kossow and his site http://www.bitsavers.org/ I got the information
 * required to start the work with this driver.
 *
 *
 *       ||
 * ||    ||  CPU-21 - main board
 * ||||--||_____________________________________________________________
 * ||||--||                                                             |
 * ||    ||                                                           _ |__
 *       ||                                                          | |   |
 * RST o-[|                                                          | |   |
 *       ||                   +-------+                              | |   |
 * ABT o-[|                   |  XTAL |                              | |   |
 *       ||                   | 50MHz |    +-----------------------+ | |   |
 * RUN   C|                   +-------+    |     MPCC              | | |   |
 * HALT  C|                   +-------+    |    R68561P            | | |VME|
 * BM    C|                   |  XTAL |    +-----------------------+ | |   |
 *       ||                   | 40MHz |    +-----------------------+ | |P1 |
 *       ||                   +-------+    |     PIT               | | |   |
 * FLMA  C|                   +-------+    |   MC68230P8           | | |   |
 * EPROM C|                   |  XTAL |    +-----------------------+ | |   |
 * 2WST  C|                   | 32MHz |                              | |   |
 * 4WST  C|                   +-------+                              | |   |
 * 6WST  C|                                                          | |   |
 * 8WST  C|                                                          |_|   |
 * 12WST C|                                   +--------------------+   |___|
 * 14WST C|                                   |    BIM             |   |
 *       ||                                   |  MC68153L          |   |
 *       ||                                   +--------------------+   |
 * CSH o-[|                                                            |
 *       ||                                                            |
 * R/H o-[|                                                            |
 *       ||                                                            |
 * DIP0   -    +----------------+                                      |
 *  .     -    |                |                                      |___
 *  .     -    |                |    +---+                            _|   |
 *  .     -    |   CPU          |    |   |+------------+             | |   |
 *  .     -    |   M68020       |    |   || 27128      |             | |   |
 *  .     -    |                |    |   || EPROM      |             | |   |
 *  .     -    |                |    |   |+------------+             | |   |
 * DIP7   -    |                |    | F |                           | |VME|
 *       ||    +----------------+    | L |+------------+             | |   |
 *       ||                          | M || 27128      |             | |P2 |
 *      [|||O  +------------+        | E || EPROM      |             | |   |
 *     +-||||  |            |        |   |+------------+             | |   |
 *     | ||||  |  FPU       |        | m |                           | |   |
 *     | ||||  |  68881     |        | e |+------------+             | |   |
 *     | ||||  |            |        | m || 27128      |             | |   |
 *     | ||||  |            |        |   || EPROM      |             | |   |
 * P4  | ||||  +------------+        | b |+------------+             | |   |
 *     | ||||                        | u |                           | |   |
 *     +-||||                        | s |+------------+             |_|   |
 *      [|||O                        |   || 27128      |               |___|
 * ||    ||                          |   || EPROM      |                |
 * ||||--||                          +---++------------+                |
 * ||||--||-------------------------------------------------------------+
 * ||
 *
 *
 *       ||
 * ||    ||  CPU-21 - slave board (SRAM-22 - connected via FLME mem bus)
 * ||||--||_____________________________________________________________
 * ||||--||                                                             |
 * ||    ||    64 x 64Kbit SRAM = 512KB                               __|__
 *       ||   +-----------+ +-----------+ +-----------+ +-----------+| |   |
 *       ||   |IMS1600    | |           | |           | |           || |   |
 *       ||   +-----------+ +-----------+ +-----------+ +-----------+| |   |
 *  RUN  C|   |           | |           | |           | |           || |   |
 *       ||   +-----------+ +-----------+ +-----------+ +-----------+| |   |
 *       ||   |           | |           | |           | |           || |   |
 *       ||   +-----------+ +-----------+ +-----------+ +-----------+| |VME|
 *  SEL0 C|   |           | |           | |           | |           || |   |
 *  SEL1 C|   +-----------+ +-----------+ +-----------+ +-----------+| |P1 |
 *       ||   |           | |           | |           | |           || |   |
 *       ||   +-----------+ +-----------+ +-----------+ +-----------+| |   |
 *       ||   |           | |           | |           | |           || |   |
 *       ||   +-----------+ +-----------+ +-----------+ +-----------+| |   |
 *       ||   |           | |           | |           | |           || |   |
 *       ||   +-----------+ +-----------+ +-----------+ +-----------+| |   |
 *       ||   |           | |           | |           | |           ||_|   |
 *       ||   +-----------+ +-----------+ +-----------+ +-----------+  |___|
 *       ||   |           | |           | |           | |           |  |
 *       ||   +-----------+ +-----------+ +-----------+ +-----------+  |
 *       ||   |           | |           | |           | |           |  |
 *       ||   +-----------+ +-----------+ +-----------+ +-----------+  |
 *       ||   |           | |           |                              |
 *       ||   +-----------+ +-----------+ +-----------+ +-----------+  |
 *       ||   |           | |           | |           | |           |  |
 *       ||   +-----------+ +-----------+ +-----------+ +-----------+  |
 *       ||                                                            |___
 *       ||                          +---++-----------+ +-----------+ _|   |
 *       ||                          |   ||           | |           || |   |
 *       ||                          |   |+-----------+ +-----------+| |   |
 *       ||                          |   ||           | |           || |   |
 *       ||                          |   |+-----------+ +-----------+| |   |
 *       ||                          | F ||           | |           || |VME|
 *       ||                          | L |+-----------+ +-----------+| |   |
 *       ||                          | M |                           | |P2 |
 *      [|||O                        | E |                           | |   |
 *     +-||||                        |   |                           | |   |
 *     | ||||                        | m |                           | |   |
 *     | ||||                        | e |                           | |   |
 *     | ||||                        | m |                           | |   |
 *     | ||||                        |   |                           | |   |
 * P8? | ||||                        | b |                           | |   |
 *     | ||||                        | u | +-----------------------+ | |   |
 *     +-||||                        | s | |      MPCC             | |_|   |
 *      [|||O                        |   | |     R68561P           |   |___|
 * ||    ||                          |   | +-----------------------+   |
 * ||||--||                          +---+                    (XTAL)   |
 * ||||--||------------------------------------------------------------+
 * ||
 *
 * History of Force Computers
 *---------------------------
 * see fccpu30.cpp
 *
 * Misc links about Force Computes and this board:
 *------------------------------------------------
 * http://bitsavers.org/pdf/forceComputers/
 *
 * Description(s)
 * -------------
 * CPU-20 has the following feature set:
 * - 68020 CPU with l6.7MHz Clock Frequency
 * - 68881 Floating Point Coprocessor with l6.7MHz Clock Frequency
 * - Static RAM 5l2Kbyte with 55ns access time
 * - 5l2Kbyte (max) of ROM/EPROM for the system
 * - 2 RS232 Multi Protocol Communication Interfaces (110-38400 Baud)
 * - Parallel Interface and Timer Chip provides local control and timer function
 * - VMXbus Primary Master Interface to p2 connector
 * - Local Interrupt handling via interrupt vectors
 * - Each VMEbus IRQ level can be enabled/disabled via software
 * - Address range for the short I/O address modifies (AM4)
 * - Address range for the standard address modifier
 * - Single level bus arbiter
 * - One level slave bus arbitration
 * - Power monitor
 * - RESET and SOFTWARE ABORT function switches
 * - Fully VMEbus, VMXbus and IEEE Pl~14 compatible
 *
 * Address Map
 * --------------------------------------------------------------------------
 *  Range                   Decscription
 * --------------------------------------------------------------------------
Basadressen av I / O-enheter:
 * 00000000-0xxFFFFF        Shared DRAM D8-D32
 * 0yy00000-FAFFFFFF        VME A32 D8-D32     yy=xx+1
 * FB000000-FBFEFFFF        VME A24 D8-D32
 * FBFF0000-FBFFFFFF        VME A16 D8-D32
 * FC000000-FCFEFFFF        VME A24 D8-D16
 * FCFF0000-FCFFFFFF        VME A16 D8-D16
 * FF800000                 MPCC
 * FF800200                 MPCC1 - on daughter board
 * FF800600                 MPCC2 - on daughter board
 * FF800800                 BIM
 * FF800C00                 PIT
 * FF800A00                 RTC
 * --------------------------------------------------------------------------
 *
 * PIT #1 hardware wiring
 * ----------------------------------------------------------
 * PA0-PA3  TBC
 * PA4-PA7
 * H1-H4
 * PB0-PB2
 * PB3-PB4
 * PB5
 * PB6-PB7
 * PC0,PC1
 * PC4,PC7
 * PC2
 * PC3
 * PC5
 * PC6
 *
 * PIT #2 hardware setup wiring
 * ----------------------------------------------------------
 * PA0-PA7  TBC
 * H1-H4
 * PB0-PB2
 * PB3-PB7
 * PC0-PC1
 * PC2
 * PC3
 * PC4
 * PC5
 * PC6
 * PC7
 *
 *---------------------------------------------------------------------------
 *  TODO:
 *  - Find accurate documentation and adjust memory map
 *  - Add PCB layout
 *  - Improve 68561 UART
 *  - Improve hookup of 68230 PIT
 *  - Add variants of boards in the CPU-20 and CPU-21 family
 *  - Add FGA, DUSCC devices and CPU-22 variants
 *
 ****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "bus/vme/vme.h"
#include "bus/vme/vme_fcisio.h"
#include "bus/vme/vme_fcscsi.h"
#include "bus/rs232/rs232.h"
#include "machine/68230pit.h"
#include "machine/68153bim.h"
#include "machine/68561mpcc.h"
#include "machine/clock.h"

#define LOG_GENERAL 0x01
#define LOG_SETUP   0x02
#define LOG_PRINTF  0x04
#define LOG_INT     0x08

#define VERBOSE 0 // (LOG_PRINTF | LOG_SETUP  | LOG_GENERAL | LOG_INT)

#define LOGMASK(mask, ...)   do { if (VERBOSE & mask) logerror(__VA_ARGS__); } while (0)
#define LOGLEVEL(mask, level, ...) do { if ((VERBOSE & mask) >= level) logerror(__VA_ARGS__); } while (0)

#define LOG(...)      LOGMASK(LOG_GENERAL, __VA_ARGS__)
#define LOGSETUP(...) LOGMASK(LOG_SETUP,   __VA_ARGS__)
#define LOGINT(...)   LOGMASK(LOG_INT,     __VA_ARGS__)

#if VERBOSE & LOG_PRINTF
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

class cpu20_state : public driver_device
{
public:
cpu20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device (mconfig, type, tag)
		, m_maincpu (*this, "maincpu")
		, m_pit (*this, "pit")
		, m_bim  (*this, "bim")
		, m_mpcc  (*this, "mpcc")
		, m_mpcc2  (*this, "mpcc2")
		, m_mpcc3  (*this, "mpcc3")
	{
	}
	DECLARE_READ32_MEMBER (bootvect_r);
	DECLARE_WRITE32_MEMBER (bootvect_w);

	DECLARE_WRITE_LINE_MEMBER(bim_irq_callback);
	uint8_t bim_irq_state;
	int bim_irq_level;

	virtual void machine_start () override;
	virtual void machine_reset () override;

	/* PIT callbacks */
	DECLARE_READ8_MEMBER (pita_r);
	DECLARE_READ8_MEMBER (pitb_r);
	DECLARE_READ8_MEMBER (pitc_r);

private:
	required_device<m68000_base_device> m_maincpu;
	required_device<pit68230_device> m_pit;
	required_device<bim68153_device> m_bim;
	required_device<mpcc68561_device> m_mpcc;
	required_device<mpcc68561_device> m_mpcc2;
	required_device<mpcc68561_device> m_mpcc3;

	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	uint32_t  *m_sysrom;
	uint32_t  m_sysram[2];
	void update_irq_to_maincpu();
};

/*
*/
static ADDRESS_MAP_START (cpu20_mem, AS_PROGRAM, 32, cpu20_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x00000000, 0x00000007) AM_ROM AM_READ  (bootvect_r)   /* ROM mirror just during reset */
	AM_RANGE (0x00000000, 0x00000007) AM_RAM AM_WRITE (bootvect_w)   /* After first write we act as RAM */
	AM_RANGE (0x00000008, 0x003fffff) AM_RAM /* RAM  installed in machine start */
	AM_RANGE (0xff040000, 0xff04ffff) AM_RAM /* RAM  installed in machine start */
	AM_RANGE (0xff000000, 0xff00ffff) AM_ROM AM_REGION("roms", 0x0000)
	AM_RANGE (0xff800000, 0xff80001f) AM_DEVREADWRITE8("mpcc", mpcc68561_device, read, write, 0xffffffff)
	AM_RANGE (0xff800200, 0xff80021f) AM_DEVREADWRITE8("mpcc2", mpcc68561_device, read, write, 0xffffffff)
//	AM_RANGE (0xff800200, 0xff8003ff) AM_DEVREADWRITE8("pit2", pit68230_device, read, write, 0xff00ff00)
	AM_RANGE (0xff800600, 0xff80061f) AM_DEVREADWRITE8("mpcc3", mpcc68561_device, read, write, 0xffffffff)
	AM_RANGE (0xff800800, 0xff80080f) AM_DEVREADWRITE8("bim", bim68153_device, read, write, 0xff00ff00)
//	AM_RANGE (0xff800a00, 0xff800a0f) AM_DEVREADWRITE8("rtc", rtc_device, read, write, 0x00ff00ff)
	AM_RANGE (0xff800c00, 0xff800dff) AM_DEVREADWRITE8("pit", pit68230_device, read, write, 0xffffffff)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START (cpu20)
INPUT_PORTS_END

/* Start it up */
void cpu20_state::machine_start ()
{
	LOGSETUP("%s\n", FUNCNAME);

	save_pointer (NAME (m_sysrom), sizeof(m_sysrom));
	save_pointer (NAME (m_sysram), sizeof(m_sysram));

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (uint32_t*)(memregion ("roms")->base());
}

void cpu20_state::machine_reset ()
{
	LOGSETUP("%s\n", FUNCNAME);

	/* Reset pointer to bootvector in ROM for bootvector handler bootvect_r */
	if (m_sysrom == &m_sysram[0]) /* Condition needed because memory map is not setup first time */
		m_sysrom = (uint32_t*)(memregion ("roms")->base());

	m_pit->h1_w(1); // signal no ACFAIL or SYSFAIL
}

#if 0
/*                                                                              setup board ID */
DRIVER_INIT_MEMBER( cpu20_state, cpu20x )      { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x50; }
DRIVER_INIT_MEMBER( cpu20_state, cpu20xa )     { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x50; }
DRIVER_INIT_MEMBER( cpu20_state, cpu20za )     { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x50; }
DRIVER_INIT_MEMBER( cpu20_state, cpu20zbe )    { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x50; }
DRIVER_INIT_MEMBER( cpu20_state, cpu20be8 )    { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x50; }
DRIVER_INIT_MEMBER( cpu20_state, cpu20be16 )   { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x50; }
DRIVER_INIT_MEMBER( cpu20_state, cpu20lite4 )  { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x50; }
DRIVER_INIT_MEMBER( cpu20_state, cpu20lite8 )  { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x50; }
DRIVER_INIT_MEMBER( cpu20_state, cpu33 )       { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x68; } // 0x60 skips FGA prompt
#endif

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0xff800000 to 0x0 at reset*/
READ32_MEMBER (cpu20_state::bootvect_r){
	LOG("%s\n", FUNCNAME);
	return m_sysrom[offset];
}

WRITE32_MEMBER (cpu20_state::bootvect_w){
	LOG("%s\n", FUNCNAME);
	m_sysram[offset % sizeof(m_sysram)] &= ~mem_mask;
	m_sysram[offset % sizeof(m_sysram)] |= (data & mem_mask);
	m_sysrom = &m_sysram[0]; // redirect all upcomming accesses to masking RAM until reset.
}

WRITE_LINE_MEMBER(cpu20_state::bim_irq_callback)
{
	LOGINT("%s(%02x)\n", FUNCNAME, state);

	bim_irq_state = state;
	bim_irq_level = m_bim->get_irq_level();
	LOGINT(" - BIM irq level  %02x\n", bim_irq_level);
	update_irq_to_maincpu();
}

void cpu20_state::update_irq_to_maincpu()
{
	LOGINT("%s()\n", FUNCNAME);
	LOGINT(" - bim_irq_level: %02x\n", bim_irq_level);
	LOGINT(" - bim_irq_state: %02x\n", bim_irq_state);
	switch (bim_irq_level & 0x07)
	{
	case 1: m_maincpu->set_input_line(M68K_IRQ_1, bim_irq_state); break;
	case 2: m_maincpu->set_input_line(M68K_IRQ_2, bim_irq_state); break;
	case 3: m_maincpu->set_input_line(M68K_IRQ_3, bim_irq_state); break;
	case 4: m_maincpu->set_input_line(M68K_IRQ_4, bim_irq_state); break;
	case 5: m_maincpu->set_input_line(M68K_IRQ_5, bim_irq_state); break;
	case 6: m_maincpu->set_input_line(M68K_IRQ_6, bim_irq_state); break;
	case 7: m_maincpu->set_input_line(M68K_IRQ_7, bim_irq_state); break;
	default: logerror("Programmatic error in %s, please report\n", FUNCNAME);
	}
}

/* 8 configuration DIP switches

 Baud   B3 B2 B1 B0
   9600  0  0  0  1
  28800  0  0  1  0
  38400  1  0  1  0
  57600  0  0  1  1

 B3: 8 bit 38400 baud

 B4:

 B5:

 B6: Auto execute FF00C0000

 B7: memory size?
*/
#define BR7N9600   0x01
#define BR7N28800  0x02
#define BR7N38400  0x06
#define BR7N57600  0x03
#define BR8N38400  0x08
#define FORCEBUG   0x30
READ8_MEMBER (cpu20_state::pita_r){
	LOG("%s\n", FUNCNAME);

	return FORCEBUG | BR7N9600;
}

/* Enabling/Disabling of VME IRQ 1-7 */
READ8_MEMBER (cpu20_state::pitb_r){
	LOG("%s\n", FUNCNAME);
	return 0xff;
}

/* VME bus release software settings (output) (ROR, RAT, RATAR, RATBCLR, RORAT, RORRAT */
READ8_MEMBER (cpu20_state::pitc_r){
	LOG("%s\n", FUNCNAME);
	return 0xff;
}

static SLOT_INTERFACE_START(fccpu20_vme_cards)
	SLOT_INTERFACE("fcisio", VME_FCISIO1)
	SLOT_INTERFACE("fcscsi", VME_FCSCSI1)
SLOT_INTERFACE_END

/*
 * Machine configuration
 */
static MACHINE_CONFIG_START (cpu20, cpu20_state)
	/* basic machine hardware */
	MCFG_CPU_ADD ("maincpu", M68020, XTAL_16MHz) /* Crytstal not verified */
	MCFG_CPU_PROGRAM_MAP (cpu20_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("bim", bim68153_device, iack)

	MCFG_VME_DEVICE_ADD("vme")
	MCFG_VME_SLOT_ADD ("vme", "vme1", fccpu20_vme_cards, nullptr)

	/* PIT Parallel Interface and Timer device, assumed strapped for on board clock */
	MCFG_DEVICE_ADD ("pit", PIT68230, XTAL_32MHz / 4) /* Crystal not verified */
	MCFG_PIT68230_PA_INPUT_CB(READ8(cpu20_state, pita_r))
	MCFG_PIT68230_PB_INPUT_CB(READ8(cpu20_state, pitb_r))
	MCFG_PIT68230_PC_INPUT_CB(READ8(cpu20_state, pitc_r))
	MCFG_PIT68230_TIMER_IRQ_CB(DEVWRITELINE("bim", bim68153_device, int2_w))

	/* BIM */
	MCFG_MC68153_ADD("bim", XTAL_32MHz / 8)
	MCFG_BIM68153_OUT_INT_CB(WRITELINE(cpu20_state, bim_irq_callback))
		/*INT0 - Abort switch */
		/*INT1 - MPCC@8.064 MHz aswell */
		/*INT2 - PI/T timer */
		/*INT3 - SYSFAIL/IRQVMX/ACFAIL/MPCC2/3 */

	/* MPCC */
#define RS232P1_TAG      "rs232p1"
#define RS232P2_TAG      "rs232p2"
#define RS232P3_TAG      "rs232p3"
	// MPCC
	MCFG_MPCC68561_ADD ("mpcc", XTAL_32MHz / 4, 0, 0)
	MCFG_MPCC_OUT_TXD_CB(DEVWRITELINE(RS232P1_TAG, rs232_port_device, write_txd))
	MCFG_MPCC_OUT_DTR_CB(DEVWRITELINE(RS232P1_TAG, rs232_port_device, write_dtr))
	MCFG_MPCC_OUT_RTS_CB(DEVWRITELINE(RS232P1_TAG, rs232_port_device, write_rts))
	MCFG_MPCC_OUT_INT_CB(DEVWRITELINE("bim", bim68153_device, int1_w))
	/* Additional MPCC sits on slave boards like SRAM-22 */
	// MPCC2
	MCFG_MPCC68561_ADD ("mpcc2", XTAL_32MHz / 4, 0, 0)
	MCFG_MPCC_OUT_TXD_CB(DEVWRITELINE(RS232P2_TAG, rs232_port_device, write_txd))
	MCFG_MPCC_OUT_DTR_CB(DEVWRITELINE(RS232P2_TAG, rs232_port_device, write_dtr))
	MCFG_MPCC_OUT_RTS_CB(DEVWRITELINE(RS232P2_TAG, rs232_port_device, write_rts))
	MCFG_MPCC_OUT_INT_CB(DEVWRITELINE("bim", bim68153_device, int3_w))
	// MPCC3
	MCFG_MPCC68561_ADD ("mpcc3", XTAL_32MHz / 4, 0, 0)
	MCFG_MPCC_OUT_TXD_CB(DEVWRITELINE(RS232P3_TAG, rs232_port_device, write_txd))
	MCFG_MPCC_OUT_DTR_CB(DEVWRITELINE(RS232P3_TAG, rs232_port_device, write_dtr))
	MCFG_MPCC_OUT_RTS_CB(DEVWRITELINE(RS232P3_TAG, rs232_port_device, write_rts))
	MCFG_MPCC_OUT_INT_CB(DEVWRITELINE("bim", bim68153_device, int3_w))

	// MPCC - RS232
	MCFG_RS232_PORT_ADD (RS232P1_TAG, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("mpcc", mpcc68561_device, write_rx))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("mpcc", mpcc68561_device, cts_w))

	// MPCC2 - RS232
	MCFG_RS232_PORT_ADD (RS232P2_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("mpcc2", mpcc68561_device, write_rx))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("mpcc2", mpcc68561_device, cts_w))

	// MPCC3 - RS232
	MCFG_RS232_PORT_ADD (RS232P3_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("mpcc3", mpcc68561_device, write_rx))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("mpcc3", mpcc68561_device, cts_w))

MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (fccpu20) /* This is an original rom dump */
	ROM_REGION32_BE(0x10000, "roms", 0)
// Boots with Board ID set to: 0x36 (FGA002 BOOT on terminal P4, "Wait until harddisk is up to speed " on terminal P1)
	ROM_LOAD32_BYTE("L.BIN",  0x000002, 0x4000, CRC (174ab801) SHA1 (0d7b8ed29d5fdd4bd2073005008120c5f20128dd))
	ROM_LOAD32_BYTE("LL.BIN", 0x000003, 0x4000, CRC (9fd9e3e4) SHA1 (e5a7c87021e6be412dd5a8166d9f62b681169eda))
	ROM_LOAD32_BYTE("U.BIN",  0x000001, 0x4000, CRC (d1afe4c0) SHA1 (b5baf9798d73632f7bb843cbc4b306c8c03f4296))
	ROM_LOAD32_BYTE("UU.BIN", 0x000000, 0x4000, CRC (b54d623b) SHA1 (49b272184a04570b09004de71fae0ed0d1bf5929))
ROM_END

/*
 * System ROM information
 *
 * xxxxxxx bootprom version xxx is released mmm dd, yyyy, coprighted by FORCE Computers Gmbh
 *
 * BIM setup: (reordered for improved reading)
 * : 0 Reg vector <- 1f
 * : 1 Reg vector <- 1c
 * : 2 Reg vector <- 1d
 * : 3 Reg vector <- 1c
 * : 0 Reg control <- 57 - Lev:7 Auto Disable:0 Int Enable:1 Vector:0 Auto Clear:1 Flag:0
 * : 1 Reg control <- 54 - Lev:4 Auto Disable:0 Int Enable:1 Vector:0 Auto Clear:1 Flag:0
 * : 2 Reg control <- 55 - Lev:5 Auto Disable:0 Int Enable:1 Vector:0 Auto Clear:1 Flag:0
 * : 3 Reg control <- 54 - Lev:4 Auto Disable:0 Int Enable:1 Vector:0 Auto Clear:1 Flag:0
 *
 * PIT setup:
 * :pit Reg 0a -> 00
 * :pit Reg 00 <- 30 - PGCR  - Mode 0, H34:enabled, H12:enabled, Sense assert H4:Lo, H3:Lo, H2:Lo, H1:Lo
 * :pit Reg 01 <- 08 - PSSR - PC4 pin activated, PC5 pin support no interrupts, H prio mode:0
 * :pit Reg 06 <- 84 - PACR
 * :pit Reg 02 <- 00 - PADDR: 00
 * :pit Reg 07 <- 84 - PBCR
 * :pit Reg 09 <- ff - PBDR
 * :pit Reg 03 <- ff - PBDDR: ff
 * :pit Reg 0c <- 07 - PCDR
 * :pit Reg 04 <- 87 - PCDDR: 87
 * :pit Reg 15 <- d8 - CPRL
 * :pit Reg 14 <- 09 - CPRM
 * :pit Reg 13 <- 00 - CPRH
 * :pit Reg 10 <- e1 - TCR - PC3 used as TOUT and PC7 used as I/O pin, Interrupts enabled
                           - PC2 used as I/O pin,CLK and x32 prescaler are used
						   - Timer reload the preload values when reaching 0 (zero)
						   - Timer is enabled
 * MPCC setup
 * : Reg 19 <- 1e - PSR2: Byte mode, 1 Stop bit, 8 bit data, ASYNC mode
 * : Reg 1c <- 8a - BRDR1: Baud Rate Divider 1
 * : Reg 1d <- 00 - BRDR1: Baud Rate Divider 2
 * : Reg 1e <- 1c - CCR: x3 Mode, TxC is output, internal RxC, ISOC
 * : Reg 1f <- 00 - ECR: No parity
 * : Reg 0d <- 00 - TIER: interrupts disabled
 * : Reg 15 <- 00 - SIER: interrupts disabled
 * : Reg 05 <- 80 - RIER: enable RDA interrupts
 * : Reg 01 <- 01 - RCR: Reset receiver command
 * : Reg 01 <- 00 - RCR: Reciver in normal operation
 * : Reg 09 <- 01 - TCR: Reset transmitter command
 * : Reg 09 <- 80 - TCR: Transmitter in normal operation
 * : Reg 11 <- c0 - SICR: Assert RTS, Assert DTR
 * : Reg 08 -> 80 - TSR: Tx FIFO has room
 * : Reg 0a <- 0a - TDR: send 0x0a to Tx FIFO... etc
 *
 * TDR outputs:
 * "Disk Controller installed
 *  Disk #0:  Header sector error = 145
 *  Disk #1:  Header sector error = 145
 *  Out of PDOS boot disk table entries.
 *  I'LL Retry them all."
 *
 */

/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT         COMPANY                   FULLNAME                FLAGS */
COMP (1986, fccpu20,      0,       0,      cpu20,          cpu20,    driver_device,      0,      "Force Computers Gmbh",   "SYS68K/CPU-20",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
