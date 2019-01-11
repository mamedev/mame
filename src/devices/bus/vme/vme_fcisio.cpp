// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  10/06/2016
 *  The ISIO board is a VME slave board that reads command and returns results through dual ported RAM to the VME bus.
 *
 * ISIO-1: page 385 http://bitsavers.informatik.uni-stuttgart.de/pdf/forceComputers/1988_Force_VMEbus_Products.pdf
 * ISIO-2: page 395 http://bitsavers.informatik.uni-stuttgart.de/pdf/forceComputers/1988_Force_VMEbus_Products.pdf
 *
 *       ||
 * ||    ||
 * ||||--||
 * ||||--|| ISIO-1 Rev U (Newer revs has two EPROMs)
 * ||    ||__________________________________________________________   ___
 *       || +---+---+---+---+---+---+---+---+ +---+ +---+ +---------+|_|   |
 * RUN   C| |16x    |   |   |   |   |   |   | |   | |   | |         || |   |
 *       || |64 or  |   |   |   |   |   |   | |   | |   | +---------+| |   |
 * R/L o-[| |256 Kb |   |   |   |   |   |   | |   | |   | |         || |   |
 *       || |SRAM   |   |   |   |   |   |   | |   | |   | +=========+| |   |
 * LOCAL C| +---+---+---+---+---+---+---+---+ |   | |   | |         || |   |
 * HALT  C| +---+---+---+---+---+---+---+---+ +---+ +---+ +---------+| |VME|
 *       || |   |   |   |   |   |   |   |   | |   | |   | |         || |   |
 *       || |   |   |   |   |   |   |   |   | |   | |   | +---------+| |P1 |
 * SEL   C| |   |   |   |   |   |   |   |   | |   | |   | |         || |   |
 * FAIL  C| |   |   |   |   |   |   |   |   | |   | |   | +---------+| |   |
 *       || +---+---+---+---+---+---+---+---+ |   | +---+ |         || |   |
 *       || +------+  +----++---+---+---+     +---+ |   | +---------+| |   |
 *       || |      |..|    ||   |   |   |   [[ [[[  |   | |         || |   |
 *  S1   C| |EPROM |..|    ||   |   |   |    +------+   | +=========+| |   |
 *  S2   C| |      |..|    ||   |   |   |    |      |   | |         ||_|   |
 *  S3   C| |      |..+----++   |   |   |    |BIM   |   | |---------+  |___|
 *  S4   C| |      | .|XTL  |   |   |   |    |68153 +---+ |         |  |
 *       || |      |..|16.0 +---+---+---+---+|      |   | +---------+  |
 *       || +------+..|MHz  |   +---+---+---+|      |   | |         |  |
 *       ||+----------+-----+---+   |   |   ||      |   | +---------+  |
 *       |||          +-----+   |   |   |   ||      |   | |         |  |
 *       ||| CPU      |XTL  |   |   |   |   ||      |   | +=========+  |
 *       ||| 68010    |14.74|   |   |   |   |+------+---+  [[[[        |
 *       |||          |MHz  +---+---+---+---+              =========   |
 *       ||+---+======+==---+===========+                  =========   |___
 *       |+---+|                        |..        +------++------+   _|   |
 *       ||   || PIT 68230              |..        |1488  ||1489  |  | |   |
 *       ||   ||                        |..        +------++------+  | |   |
 *       ||   |+------------------------+..        |1488  ||1489  |  | |   |
 *       ||   ||                        |..        +------++------+  | |   |
 *       |+---+| DUSCC 68562            |..        |75188 ||1489  |  | |VME|
 *       |+---+|                        |..        +------++------+  | |   |
 *       ||   |+------------------------+..        |75188 ||1489  |  | |P2 |
 *       ||   ||                        |..        +------++------+  | |   |
 *       ||   || DUSCC 68562            |..        |75188 ||1489  |  | |   |
 *       |+---+|                        |..        +------++------+  | |   |
 *       |+---++------------------------+..        |75188 ||1489  |  | |   |
 *       ||   ||                        |..        +------++------+  | |   |
 *       ||   || DUSCC 68562            |..        |1488  ||1489  |  | |   |
 *       ||   ||                        |..        +------++------+  | |   |
 *       ||   |+------------------------+..        |1488  ||1489  |  | |   |
 *       |+---+|                        |..        +------++------+  |_|   |
 *       ||    | DUSCC 68562            |..                =========   |___|
 * ||    ||    |                        |..                =========   |
 * ||||--||----+------------------------+------------------------------+
 * ||||--||
 * ||
 *
 * History of Force Computers
 *------------------------------------------------------------------------
 *  See fccpu30.cpp
 *
 * Description from datasheet etc
 * ------------------------------
 * ISIO-1/1A ISIO-2/2A Intelligent Serial I/O Boards
 *  - 68010 for local handling and control
 *  - 8 Channel, multi-protocol serial I/O controller board
 *  - Onboard RS-232 tranceiver. ISIO-2x: optional RS-422 transiever
 *  - ISIO-x: 128Kb ISIO-xA: 512Kb - No Wait State Dual Ported RAM
 *  - 4 fully software programmable VME bus interrupt channels
 *  - Powerful handling firmware(!)
 *  - VMEbus IEEE 1014 compatibility: A24:D16, D8, SYSFAIL (jumper)
 *  - Local watchdog timer
 *
 * Local address map - guessed/rev-enged
 * ----------------------------------------------------------
 * Address Range   Description
 * ----------------------------------------------------------
 * 000000 - 000007 Initialisation vectors from system EPROM
 * 000008 - 01FFFF Local SRAM
 * E00000 - E001FF DUSCC0
 * E20000 - E001FF DUSCC0
 * E40000 - E001FF DUSCC0
 * E60000 - E001FF DUSCC0
 * E80000 - E80DFF PI/T
 * f00000 - F70000 EPROMs
 * ----------------------------------------------------------
 *
 * VME side A24 address map - Dual ported RAM
 * ----------------------------------------------------------
 * Offset Range     Description
 * ----------------------------------------------------------
 * 000000 - 0007FF  BIM
 * 000800 - 000FFF  Status registers
 * 001000 - 0017FF  Read generates local interrupt
 * 001800 - 001FFF  Read generates local reset
 * 002000 - 007FFF  Reserved for local CPU
 * 008000 - 0080FF  16 command channels for 4 x DUSCC Rx/Tx (A/B)
 * 008100 - 01FFFF  16 data arrays for the I/O channels
 * ----------------------------------------------------------
 *
 * Interrupt sources
 * ----------------------------------------------------------
 * Description                  Device  Lvl  IRQ    VME board
 *                           /Board      Vector  Address
 * ----------------------------------------------------------
 * On board Sources
 * ----------------------------------------------------------
 *
 *  TODO:
 *  - add PIT and DUSCC interrupts
 *  - add port mapping to self test jumper
 *  - add VME bus driver
 *  - write and map a 68153 device (accessable from VME side)
 *
 *  Status: passes Self test and get stuck on no ticks for the scheduler.
 *          Schematics of the IRQ routing needed or a good trace of how the
 *          PIT and DUSCCs are hooked up to the BIM to get further.
 *
 ****************************************************************************/
#include "emu.h"
#include "vme_fcisio.h"

#include "cpu/m68000/m68000.h"
#include "machine/scnxx562.h"
#include "machine/68230pit.h"
#include "machine/68153bim.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"

//#define LOG_GENERAL (1U <<  0)
#define LOG_SETUP   (1U <<  1)

//#define VERBOSE (LOG_GENERAL | LOG_SETUP)
//#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP,  __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define TODO "VME side hookup of 68153 BIM device needed\n"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VME_FCISIO1, vme_fcisio1_card_device, "fcisio1", "Force Computer SYS68K/ISIO-1/2 Intelligent Serial I/O Board")

#define CPU_CLOCK XTAL(20'000'000) /* HCJ */
#define DUSCC_CLOCK XTAL(14'745'600) /* HCJ */

void vme_fcisio1_card_device::fcisio1_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x01ffff).ram(); /* SRAM */
	map(0x000000, 0x000007).rom().r(FUNC(vme_fcisio1_card_device::bootvect_r));       /* Vectors mapped from System EPROM */
	map(0xe00000, 0xe001ff).rw("duscc0", FUNC(duscc68562_device::read), FUNC(duscc68562_device::write)).umask16(0x00ff);
	map(0xe20000, 0xe201ff).rw("duscc1", FUNC(duscc68562_device::read), FUNC(duscc68562_device::write)).umask16(0x00ff);
	map(0xe40000, 0xe401ff).rw("duscc2", FUNC(duscc68562_device::read), FUNC(duscc68562_device::write)).umask16(0x00ff);
	map(0xe60000, 0xe601ff).rw("duscc3", FUNC(duscc68562_device::read), FUNC(duscc68562_device::write)).umask16(0x00ff);
	map(0xe80000, 0xe80dff).rw("pit", FUNC(pit68230_device::read), FUNC(pit68230_device::write)).umask16(0x00ff);
	map(0xf00000, 0xf7ffff).rom(); /* System EPROM Area 32Kb DEBUGGER supplied */
//  AM_RANGE (0xc40000, 0xc800ff) AM_READWRITE8 (not_implemented_r, not_implemented_w, 0xffff)  /* Dummy mapping af address area to display message */
}

/* ROM definitions */
ROM_START (fcisio1)
	ROM_REGION (0x1000000, "maincpu", 0)

/* ISIO ROM:s v2.1 information
 * PIT setup sequence
 *     00 -> REG_PGCR
 *     18 -> REG_PSRR
 *     0f -> Reg PADDR
 *     0f -> REG_PBDDR
 *     fa -> REG_PACR
 *     0f -> REG_PADDR
 *     fa -> REG_PBCR
 *     ff -> REG_PBDR
 *     0f -> REG_PBDDR
 *     10 -> REG_PGCR
 *     ff -> REG_PCDR
 *     17 -> REG_PCDDR
 *     40 -> Reg PIVR
 *     00 -> REG_TCR   - timer disabled, all C pins, use preload, CLK and prescaler are used
 *     a0 -> REG_TCR   - timer disabled, The dual-function pin PC3/TOUT carries the TOUTfunction and is used as a timer interrupt request
 *                       output. The timer interrupt is enabled ; thus, the pin is low when the timer ZDS status bit is one. The dual-function
 *                       pin PC7/TIACK carries the TIACK function and is used as a timer interrupt acknowledge input.
 *     00 -> Reg 0x12
 *     00 -> REG_CPRH
 *     09 -> REG_CPRM
 *     00 -> REG_CPRL
 *     00 -> Reg 0x16
 *     00 -> Reg 0x17
 *     09 -> Reg 0x18
 *     00 -> Reg 0x19
 *     1d -> Reg TIVR
 *     0f <- REG_PBDR
 *     0e -> REG_PBDR
 *     0f <- REG_PBDR
 *     0f <- REG_PBDR
 *     0d -> REG_PDBR
 *     0f <- REG_PBDR
 *     0f <- REG_PBDR
 *     0f <- REG_PBDR
 *     0f <- REG_PBDR
 *     0f <- REG_PBDR
 *     0f <- REG_PBDR
 *     0f <- REG_PBDR
 *     0f <- REG_PBDR
 *     0f <- REG_PBDR
 *     0f -> REG_PBDR
 *     0f <- REG_PBDR
 *     0b -> REG_PBDR
 *     0f <- REG_PBDR
 *     0f <- REG_PBDR
 *     0f -> REG_PBDR
 *     0f <- REG_PBDR
 *     0f -> REG_PBDR
 *     0f <- REG_PBDR
 *     00 <- REG_PCDR
 *     00 -> REG_PCDR
 * ------- repeated 16 times -------------------
 *     a1 -> REG_TCR   - timer enabled, The dual-function pin PC3/TOUT carries the TOUTfunction and is used as a timer interrupt request
 *                       output. The timer interrupt is enabled ; thus, the pin is low when the timer ZDS status bit is one. The dual-function
 *                       pin PC7/TIACK carries the TIACK function and is used as a timer interrupt acknowledge input.
 *     ?? <- Reg 0x0c
 * ---------------------------------------------
 *
 * DUSCC0 channel A setup sequence
 *  0f 00 -> REG_CCR    - Reset Tx
 *  0f 40 -> REG_CCR    - Reset Rx
 *  00 07 -> REG_CMR1   - Async mode
 *  01 38 -> REG_CMR2   - Normal polled or interrupt mode, no DMA
 *  02 00 -> REG_S1R    - SYN1, Secondary Address 1 Register, 0 = no sync
 *  03 00 -> REG_S2R    - SYN2, only used in COP dual Sync mode but alao 0 = no sync
 *  04 7F -> REG_TPR    - Tx 8 bits, CTS and RTS, 1 STOP bit
 *  05 3d -> REG_TTR    - Tx BRG 9600 (assuming a 14.7456 crystal)
 *  06 1b -> REG_RPR    - Rx RTS, 8 bits, no DCD, no parity
 *  07 2d -> REG_RTR    - Rx BRG 9600 (assuming a 14.7456 crystal)
 *  0b e1 -> REG_OMR    - RTS high, OUT1 = OUT2 = high, RxRdy asserted for each character,
 *                        TxRdy asserted on threshold, Same Tx Residual Character Length as for REG_TPR
 *  0a 00 -> REG_CTCR   - Counter/Timer control register 00 = Zero Det Int: disabled, Zero Det Control: preset,
 *                        Output Control: square, Prescaler: 1, Clock Source: RTxC pin
 *  09 00 -> REG_CTPRL  - Counter/Timer Prescaler Register Low = 0
 *  08 00 -> REG_CTPRH  - Counter/Timer Prescaler Register High = 0
 *  0f 00 -> REG_CCR    - Reset Tx
 *  0f 02 -> REG_CCR    - Enable Tx
 *  0f 40 -> REG_CCR    - Reset Rx
 *  0f 42 -> REG_CCR    - Enable Rx
 *  0f 02 -> REG_CCR    - Enable Tx
 *  0f 42 -> REG_CCR    - Enable Rx
 *  0e 27 -> REG_PCR    - TRxC = RxCLK 1x, RTxC is input, RTS, GPO2, crystal oscillator connected to X2
 *  1c 10 -> REG_IER    - Interrupt Enable Register: RxRdy generates interrupt
 *  ... chan B setup with same data....
 * ---- DUSCC0 to DUSCC3, setup with same data except at the end of each setup:
 *  1e 1c -> DUSCC0 REG_IVR -
 *  1e 1b -> DUSCC1 REG_IVR
 *  1e 1a -> DUSCC2 REG_IVR
 *  1e 19 -> DUSCC3 REG_IVR
 */
	ROM_LOAD16_BYTE ("isio-1_v2.1_l.bin", 0xf00001, 0x4000, CRC (0d47d80f) SHA1 (541b55966f464c1cf686e36998650720950a2242))
	ROM_LOAD16_BYTE ("isio-1_v2.1_u.bin", 0xf00000, 0x4000, CRC (67986768) SHA1 (215f7ff90d9dbe2bea54510e3722fb33d4e54193))
ROM_END

void vme_fcisio1_card_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	M68010(config, m_maincpu, CPU_CLOCK / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &vme_fcisio1_card_device::fcisio1_mem);

	/* DUSCC channels */
#define RS232P1_TAG      "rs232p1"
#define RS232P2_TAG      "rs232p2"
#define RS232P3_TAG      "rs232p3"
#define RS232P4_TAG      "rs232p4"
#define RS232P5_TAG      "rs232p5"
#define RS232P6_TAG      "rs232p6"
#define RS232P7_TAG      "rs232p7"
#define RS232P8_TAG      "rs232p8"

	DUSCC68562(config, m_duscc0, DUSCC_CLOCK);
	m_duscc0->configure_channels(0, 0, 0, 0);
	/* Port 1 on DUSCC 0 Port A */
	m_duscc0->out_txda_callback().set(RS232P1_TAG, FUNC(rs232_port_device::write_txd));
	m_duscc0->out_dtra_callback().set(RS232P1_TAG, FUNC(rs232_port_device::write_dtr));
	m_duscc0->out_rtsa_callback().set(RS232P1_TAG, FUNC(rs232_port_device::write_rts));
	/* Port 2 on DUSCC 0 Port B */
	m_duscc0->out_txdb_callback().set(RS232P2_TAG, FUNC(rs232_port_device::write_txd));
	m_duscc0->out_dtrb_callback().set(RS232P2_TAG, FUNC(rs232_port_device::write_dtr));
	m_duscc0->out_rtsb_callback().set(RS232P2_TAG, FUNC(rs232_port_device::write_rts));
	/* RS232 for DUSCC 0 */
	rs232_port_device &rs232p1(RS232_PORT(config, RS232P1_TAG, default_rs232_devices, "terminal"));
	rs232p1.rxd_handler().set(m_duscc0, FUNC(duscc68562_device::rxa_w));
	rs232p1.cts_handler().set(m_duscc0, FUNC(duscc68562_device::ctsa_w));

	rs232_port_device &rs232p2(RS232_PORT(config, RS232P2_TAG, default_rs232_devices, nullptr));
	rs232p2.rxd_handler().set(m_duscc0, FUNC(duscc68562_device::rxb_w));
	rs232p2.cts_handler().set(m_duscc0, FUNC(duscc68562_device::ctsb_w));

	DUSCC68562(config, m_duscc1, DUSCC_CLOCK);
	m_duscc1->configure_channels(0, 0, 0, 0);
	/* Port 3 on DUSCC 1 Port A */
	m_duscc1->out_txda_callback().set(RS232P3_TAG, FUNC(rs232_port_device::write_txd));
	m_duscc1->out_dtra_callback().set(RS232P3_TAG, FUNC(rs232_port_device::write_dtr));
	m_duscc1->out_rtsa_callback().set(RS232P3_TAG, FUNC(rs232_port_device::write_rts));
	/* Port 4 on DUSCC 1 Port B */
	m_duscc1->out_txdb_callback().set(RS232P4_TAG, FUNC(rs232_port_device::write_txd));
	m_duscc1->out_dtrb_callback().set(RS232P4_TAG, FUNC(rs232_port_device::write_dtr));
	m_duscc1->out_rtsb_callback().set(RS232P4_TAG, FUNC(rs232_port_device::write_rts));
	/* RS232 for DUSCC 1 */
	rs232_port_device &rs232p3(RS232_PORT(config, RS232P3_TAG, default_rs232_devices, nullptr));
	rs232p3.rxd_handler().set(m_duscc1, FUNC(duscc68562_device::rxa_w));
	rs232p3.cts_handler().set(m_duscc1, FUNC(duscc68562_device::ctsa_w));

	rs232_port_device &rs232p4(RS232_PORT(config, RS232P4_TAG, default_rs232_devices, nullptr));
	rs232p4.rxd_handler().set(m_duscc1, FUNC(duscc68562_device::rxb_w));
	rs232p4.cts_handler().set(m_duscc1, FUNC(duscc68562_device::ctsb_w));

	DUSCC68562(config, m_duscc2, DUSCC_CLOCK);
	m_duscc2->configure_channels(0, 0, 0, 0);
	/* Port 5 on DUSCC 2 Port A */
	m_duscc2->out_txda_callback().set(RS232P5_TAG, FUNC(rs232_port_device::write_txd));
	m_duscc2->out_dtra_callback().set(RS232P5_TAG, FUNC(rs232_port_device::write_dtr));
	m_duscc2->out_rtsa_callback().set(RS232P5_TAG, FUNC(rs232_port_device::write_rts));
	/* Port 6 on DUSCC 2 Port B */
	m_duscc2->out_txdb_callback().set(RS232P6_TAG, FUNC(rs232_port_device::write_txd));
	m_duscc2->out_dtrb_callback().set(RS232P6_TAG, FUNC(rs232_port_device::write_dtr));
	m_duscc2->out_rtsb_callback().set(RS232P6_TAG, FUNC(rs232_port_device::write_rts));
	/* RS232 for DUSCC 2 */
	rs232_port_device &rs232p5(RS232_PORT(config, RS232P5_TAG, default_rs232_devices, nullptr));
	rs232p5.rxd_handler().set(m_duscc2, FUNC(duscc68562_device::rxa_w));
	rs232p5.cts_handler().set(m_duscc2, FUNC(duscc68562_device::ctsa_w));

	rs232_port_device &rs232p6(RS232_PORT(config, RS232P6_TAG, default_rs232_devices, nullptr));
	rs232p6.rxd_handler().set(m_duscc2, FUNC(duscc68562_device::rxb_w));
	rs232p6.cts_handler().set(m_duscc2, FUNC(duscc68562_device::ctsb_w));

	DUSCC68562(config, m_duscc3, DUSCC_CLOCK);
	m_duscc3->configure_channels(0, 0, 0, 0);
	/* Port 7 on DUSCC 3 Port A */
	m_duscc3->out_txda_callback().set(RS232P7_TAG, FUNC(rs232_port_device::write_txd));
	m_duscc3->out_dtra_callback().set(RS232P7_TAG, FUNC(rs232_port_device::write_dtr));
	m_duscc3->out_rtsa_callback().set(RS232P7_TAG, FUNC(rs232_port_device::write_rts));
	/* Port 8 on DUSCC 3 Port B */
	m_duscc3->out_txdb_callback().set(RS232P8_TAG, FUNC(rs232_port_device::write_txd));
	m_duscc3->out_dtrb_callback().set(RS232P8_TAG, FUNC(rs232_port_device::write_dtr));
	m_duscc3->out_rtsb_callback().set(RS232P8_TAG, FUNC(rs232_port_device::write_rts));
	/* RS232 for DUSCC 4 */
	rs232_port_device &rs232p7(RS232_PORT(config, RS232P7_TAG, default_rs232_devices, nullptr));
	rs232p7.rxd_handler().set(m_duscc3, FUNC(duscc68562_device::rxa_w));
	rs232p7.cts_handler().set(m_duscc3, FUNC(duscc68562_device::ctsa_w));

	rs232_port_device &rs232p8(RS232_PORT(config, RS232P8_TAG, default_rs232_devices, nullptr));
	rs232p8.rxd_handler().set(m_duscc3, FUNC(duscc68562_device::rxb_w));
	rs232p8.cts_handler().set(m_duscc3, FUNC(duscc68562_device::ctsb_w));

	PIT68230(config, m_pit, XTAL(20'000'000) / 2);
	m_pit->pb_in_callback().set(FUNC(vme_fcisio1_card_device::config_rd));

	MC68153(config, m_bim, XTAL(20'000'000) / 2);
}

const tiny_rom_entry *vme_fcisio1_card_device::device_rom_region() const
{
	LOG("%s\n", FUNCNAME);
	return ROM_NAME( fcisio1 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************
vme_fcisio1_card_device::vme_fcisio1_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_maincpu (*this, "maincpu")
	, m_duscc0(*this, "duscc0")
	, m_duscc1(*this, "duscc1")
	, m_duscc2(*this, "duscc2")
	, m_duscc3(*this, "duscc3")
	, m_pit (*this, "pit")
	, m_bim (*this, "bim")
{
	LOG("%s\n", FUNCNAME);
}

vme_fcisio1_card_device::vme_fcisio1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vme_fcisio1_card_device(mconfig, VME_FCISIO1, tag, owner, clock)
{
}

/* Start it up */
void vme_fcisio1_card_device::device_start()
{
	LOG("%s\n", FUNCNAME);
	set_vme_device();

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (uint16_t*)(memregion ("maincpu")->base () + 0xf00000);

#if 0 // TODO: Setup VME access handlers for shared memory area
	uint32_t base = 0xFFFF5000;
	m_vme->install_device(base + 0, base + 1, // Channel B - Data
							 read8_delegate(FUNC(z80sio_device::db_r),  subdevice<z80sio_device>("pit")), write8_delegate(FUNC(z80sio_device::db_w), subdevice<z80sio_device>("pit")), 0x00ff);
	m_vme->install_device(base + 2, base + 3, // Channel B - Control
							 read8_delegate(FUNC(z80sio_device::cb_r),  subdevice<z80sio_device>("pit")), write8_delegate(FUNC(z80sio_device::cb_w), subdevice<z80sio_device>("pit")), 0x00ff);
#endif

}

void vme_fcisio1_card_device::device_reset()
{
	LOG("%s\n", FUNCNAME);
}

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0x80000 to 0x0 */
READ16_MEMBER (vme_fcisio1_card_device::bootvect_r){
	return m_sysrom [offset];
}

READ8_MEMBER (vme_fcisio1_card_device::not_implemented_r){
	static int been_here = 0;
	if (!been_here++){
		logerror(TODO);
		printf(TODO);
	}
	return (uint8_t) 0;
}

WRITE8_MEMBER (vme_fcisio1_card_device::not_implemented_w){
	static int been_here = 0;
	if (!been_here++){
		logerror(TODO);
		printf(TODO);
	}
	return;
}

// TODO: Get a manual to understand the config options for real
READ8_MEMBER (vme_fcisio1_card_device::config_rd){
	uint8_t ret = 0;
	LOG("%s\n", FUNCNAME);

	// Port B bit #7, 0x80 Self test bit, choose either of these two lines
	ret &= ~0x80; // 0 = selftest
	//  ret |=  0x80; // 1 = no selftest

	return ret;
}

// This info isn't kept in a card driver atm so storing it as a comment for later use
//      YEAR  NAME           PARENT  COMPAT  MACHINE       INPUT    CLASS             INIT COMPANY                  FULLNAME           FLAGS
//COMP( 1986, fcisio1,       0,      0,      fcisio1,      fcisio1, driver_device,     0,  "Force Computers Gmbh",  "SYS68K/ISIO-1",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
