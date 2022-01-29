// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Force SYS68K CPU-20 VME SBC imaginary one (two) slot chassi
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
 * FLME  C|                   +-------+    |   MC68230P8           | | |   |
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
 * ||         NOTE: Variants also available with 2MB and/or DRAM
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
 * 00000000-0xxFFFFF        Shared DRAM D8-D32
 * 0yy00000-FAFFFFFF        VME A32 D8-D32     yy=xx+1
 * FB000000-FBFEFFFF        VME A24 D8-D32
 * FBFF0000-FBFFFFFF        VME A16 D8-D32
 * FC000000-FCFEFFFF        VME A24 D8-D16
 * FCFF0000-FCFFFFFF        VME A16 D8-D16
 * FF800000                 MPCC
 * FF800200                 MPCC2 - on daughter board
 * FF800600                 MPCC3 - on daughter board
 * FF800800                 BIM
 * FF800C00                 PIT
 * FF800A00                 RTC
 *
 *---------------------------------------------------------------------------
 *  TODO:
 *  - Find accurate documentation and adjust memory map for Y boards
 *  - Improve 68561 UART
 *  - Improve hookup of 68230 PIT
 *    - ABORT switch
 *    - Enable/Disable VME interrupts
 *    - Interrupts: Timer, ACFAIL, SYSFAIL and VMX irq
 *    - Memory config
 *    - Board ID
 *  - Add FGA, DUSCC devices and CPU-22 variants
 *  - Add FLME bus for memory expansions and optional extra MPCC
 *  - Add VMX bus on the P2 connector
 *  - Enable FPU
 *  - Add user EPROM sockets as cartridge interface enabling softlists
 *  NOT PLANNED:
 *  - VME bus arbiter as MAME always gets the bus
 ****************************************************************************/
#include "emu.h"
#include "vme_fccpu20.h"

#include "cpu/m68000/m68000.h"
#include "bus/rs232/rs232.h"
#include "machine/68230pit.h"
#include "machine/68153bim.h"
#include "machine/68561mpcc.h"
#include "machine/clock.h"

//#define LOG_GENERAL (1U <<  0)
#define LOG_SETUP   (1U <<  1)
#define LOG_INT     (1U <<  2)

//#define VERBOSE (LOG_GENERAL | LOG_SETUP | LOG_INT)
//#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,   __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VME_FCCPU20,   vme_fccpu20_card_device,   "fccpu20",   "Force Computer SYS68K/CPU-20 CPU Board")
DEFINE_DEVICE_TYPE(VME_FCCPU21S,  vme_fccpu21s_card_device,  "fccpu21s",  "Force Computer SYS68K/CPU-21S CPU Board")
DEFINE_DEVICE_TYPE(VME_FCCPU21,   vme_fccpu21_card_device,   "fccpu21",   "Force Computer SYS68K/CPU-21 CPU Board")
DEFINE_DEVICE_TYPE(VME_FCCPU21A,  vme_fccpu21a_card_device,  "fccpu21a",  "Force Computer SYS68K/CPU-21A CPU Board")
DEFINE_DEVICE_TYPE(VME_FCCPU21YA, vme_fccpu21ya_card_device, "fccpu21ya", "Force Computer SYS68K/CPU-21YA CPU Board")
DEFINE_DEVICE_TYPE(VME_FCCPU21B,  vme_fccpu21b_card_device,  "fccpu21b",  "Force Computer SYS68K/CPU-21B CPU Board")
DEFINE_DEVICE_TYPE(VME_FCCPU21YB, vme_fccpu21yb_card_device, "fccpu21yb", "Force Computer SYS68K/CPU-21YB CPU Board")

#define CLOCK50 XTAL(50'000'000) /* HCJ */
#define CLOCK40 XTAL(40'000'000) /* HCJ */
#define CLOCK32 XTAL(32'000'000) /* HCJ */

void vme_fccpu20_device::cpu20_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x00000007).ram().w(FUNC(vme_fccpu20_device::bootvect_w));   /* After first write we act as RAM */
	map(0x00000000, 0x00000007).rom().r(FUNC(vme_fccpu20_device::bootvect_r));   /* ROM mirror just during reset */
	map(0x00000008, 0x0007ffff).ram(); /* Local SRAM */
	map(0x00080000, 0x000fffff).ram(); /* SRAM-22 installed */
	map(0xff040000, 0xff04ffff).ram();
	map(0xff000000, 0xff00ffff).rom().region("roms", 0x0000);
	map(0xff800000, 0xff80001f).rw("mpcc", FUNC(mpcc68561_device::read), FUNC(mpcc68561_device::write));
	map(0xff800200, 0xff80021f).rw("mpcc2", FUNC(mpcc68561_device::read), FUNC(mpcc68561_device::write));
	map(0xff800600, 0xff80061f).rw("mpcc3", FUNC(mpcc68561_device::read), FUNC(mpcc68561_device::write));
	map(0xff800800, 0xff80080f).rw("bim", FUNC(bim68153_device::read), FUNC(bim68153_device::write)).umask32(0xff00ff00);
	map(0xff800c00, 0xff800dff).rw("pit", FUNC(pit68230_device::read), FUNC(pit68230_device::write));
}


static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

void vme_fccpu20_device::cpu_space_map(address_map &map)
{
	map(0xfffffff2, 0xffffffff).lr16(NAME([this](offs_t offset) -> u16 { return m_bim->iack(offset+1); }));
}

void vme_fccpu20_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	M68020(config, m_maincpu, CLOCK50 / 3); /* Crytstal verified from picture HCI */
	m_maincpu->set_addrmap(AS_PROGRAM, &vme_fccpu20_device::cpu20_mem);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &vme_fccpu20_device::cpu_space_map);

	/* PIT Parallel Interface and Timer device, assumed strapped for on board clock */
	PIT68230(config, m_pit, CLOCK32 / 4); /* Crystal not verified */
	m_pit->pa_in_callback().set(FUNC(vme_fccpu20_device::pita_r));
	m_pit->pb_in_callback().set(FUNC(vme_fccpu20_device::pitb_r));
	m_pit->pc_in_callback().set(FUNC(vme_fccpu20_device::pitc_r));
	m_pit->timer_irq_callback().set("bim", FUNC(bim68153_device::int2_w));

	/* BIM */
	bim68153_device &bim(MC68153(config, "bim", CLOCK32 / 8));
	bim.out_int_callback().set(FUNC(vme_fccpu20_device::bim_irq_callback));
		/*INT0 - Abort switch */
		/*INT1 - MPCC@8.064 MHz aswell */
		/*INT2 - PI/T timer */
		/*INT3 - SYSFAIL/IRQVMX/ACFAIL/MPCC2/3 */

	/* MPCC */
#define RS232P1_TAG      "rs232p1"
#define RS232P2_TAG      "rs232p2"
#define RS232P3_TAG      "rs232p3"
	mpcc68561_device &mpcc(MPCC68561(config, "mpcc", CLOCK32 / 4, 0, 0));
	mpcc.out_txd_cb().set(RS232P1_TAG, FUNC(rs232_port_device::write_txd));
	mpcc.out_dtr_cb().set(RS232P1_TAG, FUNC(rs232_port_device::write_dtr));
	mpcc.out_rts_cb().set(RS232P1_TAG, FUNC(rs232_port_device::write_rts));
	mpcc.out_int_cb().set("bim", FUNC(bim68153_device::int1_w));

	/* Additional MPCC sits on FLME boards like SRAM-22,
	   TODO: install MPCC2/MPCC3 in FLME slot device */
	mpcc68561_device &mpcc2(MPCC68561(config, "mpcc2", CLOCK32 / 4, 0, 0));
	mpcc2.out_txd_cb().set(RS232P2_TAG, FUNC(rs232_port_device::write_txd));
	mpcc2.out_dtr_cb().set(RS232P2_TAG, FUNC(rs232_port_device::write_dtr));
	mpcc2.out_rts_cb().set(RS232P2_TAG, FUNC(rs232_port_device::write_rts));
	mpcc2.out_int_cb().set("bim", FUNC(bim68153_device::int3_w));

	mpcc68561_device &mpcc3(MPCC68561(config, "mpcc3", CLOCK32 / 4, 0, 0));
	mpcc3.out_txd_cb().set(RS232P3_TAG, FUNC(rs232_port_device::write_txd));
	mpcc3.out_dtr_cb().set(RS232P3_TAG, FUNC(rs232_port_device::write_dtr));
	mpcc3.out_rts_cb().set(RS232P3_TAG, FUNC(rs232_port_device::write_rts));
	mpcc3.out_int_cb().set("bim", FUNC(bim68153_device::int3_w));

	// MPCC - RS232
	rs232_port_device &rs232p1(RS232_PORT(config, RS232P1_TAG, default_rs232_devices, "terminal"));
	rs232p1.rxd_handler().set(m_mpcc, FUNC(mpcc68561_device::write_rx));
	rs232p1.cts_handler().set(m_mpcc, FUNC(mpcc68561_device::cts_w));
	rs232p1.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	// MPCC2 - RS232
	rs232_port_device &rs232p2(RS232_PORT(config, RS232P2_TAG, default_rs232_devices, nullptr));
	rs232p2.rxd_handler().set(m_mpcc2, FUNC(mpcc68561_device::write_rx));
	rs232p2.cts_handler().set(m_mpcc2, FUNC(mpcc68561_device::cts_w));

	// MPCC3 - RS232
	rs232_port_device &rs232p3(RS232_PORT(config, RS232P3_TAG, default_rs232_devices, nullptr));
	rs232p3.rxd_handler().set(m_mpcc3, FUNC(mpcc68561_device::write_rx));
	rs232p3.cts_handler().set(m_mpcc3, FUNC(mpcc68561_device::cts_w));
}

void vme_fccpu20_card_device::device_add_mconfig(machine_config &config)
{
	vme_fccpu20_device::device_add_mconfig(config);
}

// SYS68K/CPU-21S Part No.1 01 041 - 68020 CPU board + FPU 68881 at 12.5 MHz, 512 KB RAM
void vme_fccpu21s_card_device::device_add_mconfig(machine_config &config)
{
	vme_fccpu20_device::device_add_mconfig(config);

	m_maincpu->set_clock(CLOCK50 / 4);
}

// SYS68K/CPU-21 Part No.1 01 001 - 68020 CPU board (CPU-20) + FPU 68881 at 16.7 MHz, 512 KB RAM
void vme_fccpu21_card_device::device_add_mconfig(machine_config &config)
{
	vme_fccpu20_device::device_add_mconfig(config);

	m_maincpu->set_clock(CLOCK50 / 3);
}

// SYS68K/CPU-21A Part No.1 01 011 - 68020 CPU board + FPU 68881 at 20 MHz, 512 KB RAM
void vme_fccpu21a_card_device::device_add_mconfig(machine_config &config)
{
	vme_fccpu20_device::device_add_mconfig(config);

	m_maincpu->set_clock(CLOCK40 / 2);
}

// SYS68K/CPU-21YA Part No.1 01 061 - 68020 CPU board + FPU 68881 at 20 MHz, 2048 KB RAM
void vme_fccpu21ya_card_device::device_add_mconfig(machine_config &config)
{
	vme_fccpu20_device::device_add_mconfig(config);

	m_maincpu->set_clock(CLOCK40 / 2);
}

// SYS68K/CPU-21B Part No.1 01 021 - 68020 CPU board + FPU 68881 at 25 MHz, 512 KB RAM
void vme_fccpu21b_card_device::device_add_mconfig(machine_config &config)
{
	vme_fccpu20_device::device_add_mconfig(config);

	m_maincpu->set_clock(CLOCK50 / 2);
}

// SYS68K/CPU-21YB Part No.1 01 071 - 68020 CPU board + FPU 68881 at 25 MHz, 2048 KB RAM
void vme_fccpu21yb_card_device::device_add_mconfig(machine_config &config)
{
	vme_fccpu20_device::device_add_mconfig(config);

	m_maincpu->set_clock(CLOCK50 / 2);
}


//**************************************************************************
//  Base Device
//**************************************************************************
vme_fccpu20_device::vme_fccpu20_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, fc_board_t board_id) :
	device_t(mconfig, type, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_maincpu (*this, "maincpu")
	, m_pit (*this, "pit")
	, m_bim  (*this, "bim")
	, m_mpcc  (*this, "mpcc")
	, m_mpcc2  (*this, "mpcc2")
	, m_mpcc3  (*this, "mpcc3")
	, m_board_id(board_id)
{
	LOG("%s\n", FUNCNAME);
}

//**************************************************************************
//  Card Devices
//**************************************************************************
vme_fccpu20_card_device::vme_fccpu20_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: vme_fccpu20_card_device(mconfig, VME_FCCPU20, tag, owner, clock)
{
	LOG("%s %s\n", tag, FUNCNAME);
}


vme_fccpu21s_card_device::vme_fccpu21s_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vme_fccpu21s_card_device(mconfig, VME_FCCPU21S, tag, owner, clock)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

vme_fccpu21_card_device::vme_fccpu21_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vme_fccpu21_card_device(mconfig, VME_FCCPU21, tag, owner, clock)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

vme_fccpu21a_card_device::vme_fccpu21a_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vme_fccpu21a_card_device(mconfig, VME_FCCPU21A, tag, owner, clock)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

// TODO: Change to 2MB on board RAM and move FLME memory and find/verify memory map
vme_fccpu21ya_card_device::vme_fccpu21ya_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vme_fccpu21ya_card_device(mconfig, VME_FCCPU21YA, tag, owner, clock)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

vme_fccpu21b_card_device::vme_fccpu21b_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vme_fccpu21b_card_device(mconfig, VME_FCCPU21B, tag, owner, clock)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

// TODO: Change to 2MB on board RAM and move FLME memory and find/verify memory map
vme_fccpu21yb_card_device::vme_fccpu21yb_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vme_fccpu21yb_card_device(mconfig, VME_FCCPU21YB, tag, owner, clock)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

/* Start it up */
void vme_fccpu20_device::device_start()
{
	LOG("%s\n", FUNCNAME);

	save_pointer (NAME (m_sysrom), sizeof(m_sysrom));
	save_pointer (NAME (m_sysram), sizeof(m_sysram));
	//  save_item(NAME(m_board_id)); // TODO: Save this "non base type" item

	/* TODO: setup this RAM from (not yet) optional SRAM-2x board and also support 2MB versions */
	//m_maincpu->space(AS_PROGRAM).install_ram(0x80000, m_ram->size() + 0x7ffff, m_ram->pointer());

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (uint32_t*)(memregion ("roms")->base());

#if 0 // TODO: Setup VME access handlers for shared memory area
	uint32_t base = 0xFFFF5000;
	m_vme->install_device(base + 0, base + 1, // Channel B - Data
			read8_delegate(*subdevice<z80sio_device>("pit"), FUNC(z80sio_device::db_r)), write8_delegate(*subdevice<z80sio_device>("pit"), FUNC(z80sio_device::db_w)), 0x00ff);
	m_vme->install_device(base + 2, base + 3, // Channel B - Control
			read8_delegate(*subdevice<z80sio_device>("pit"), FUNC(z80sio_device::cb_r)), write8_delegate(*subdevice<z80sio_device>("pit"), FUNC(z80sio_device::cb_w)), 0x00ff);
#endif
}

enum
{
	TIMER_ID_BUS_GRANT
};

void vme_fccpu20_device::device_reset()
{
	LOG("%s\n", FUNCNAME);

	/* We need to delay the static bus grant signal until we have it from the VME interface or MAME supports bus arbitration */
	m_arbiter_start = timer_alloc(TIMER_ID_BUS_GRANT);
	m_arbiter_start->adjust(attotime::from_msec(10), TIMER_ID_BUS_GRANT, attotime::never);
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------
void vme_fccpu20_device::device_timer (emu_timer &timer, device_timer_id id, int param)
{
	switch(id)
	{
	case TIMER_ID_BUS_GRANT:
		m_pit->h1_w(ASSERT_LINE); // Grant bus always
		break;
	default:
		LOG("Unhandled Timer ID %d\n", id);
		break;
	}
}

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0xff800000 to 0x0 at reset*/
uint32_t vme_fccpu20_device::bootvect_r(offs_t offset)
{
	LOG("%s\n", FUNCNAME);
	return m_sysrom[offset];
}

void vme_fccpu20_device::bootvect_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s\n", FUNCNAME);
	m_sysram[offset % std::size(m_sysram)] &= ~mem_mask;
	m_sysram[offset % std::size(m_sysram)] |= (data & mem_mask);
	m_sysrom = &m_sysram[0]; // redirect all upcoming accesses to masking RAM until reset.
}

WRITE_LINE_MEMBER(vme_fccpu20_device::bim_irq_callback)
{
	LOGINT("%s(%02x)\n", FUNCNAME, state);

	bim_irq_state = state;
	bim_irq_level = m_bim->get_irq_level();
	LOGINT(" - BIM irq level %s\n", bim_irq_level == CLEAR_LINE ? "Cleared" : "Asserted");
	update_irq_to_maincpu();
}

void vme_fccpu20_device::update_irq_to_maincpu()
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
   9600  0  0  0  1  7 bits
  28800  0  0  1  0  7 bits
  38400  1  0  1  0  8 bits
  57600  0  0  1  1  7 bits

 B3: 8 bit 38400 baud
 B4/B5: Both set boots FORCEbug
 B6: Auto execute FF00C0000
 B7: memory size?
*/
/* PIT Port definitions */
#define BR7N9600   0x01
#define BR7N28800  0x02
#define BR7N38400  0x06
#define BR7N57600  0x03
#define BR8N38400  0x08
#define FORCEBUG   0x30

uint8_t vme_fccpu20_device::pita_r()
{
	LOG("%s\n", FUNCNAME);
	return FORCEBUG | BR7N9600;
}

/* Enabling/Disabling of VME IRQ 1-7 */
uint8_t vme_fccpu20_device::pitb_r()
{
	LOG("%s\n", FUNCNAME);
	return 0xff;
}

/* VME board ID bit and bus release software settings (output) (ROR, RAT, RATAR, RATBCLR, RORAT, RORRAT */
/* Bit 4 is bus available */
uint8_t vme_fccpu20_device::pitc_r()
{
	uint8_t board_id = 0;

	LOG("%s Board id:%02x\n", FUNCNAME, m_board_id);

	switch (m_board_id)
	{
	case cpu20:
		board_id = CPU20;
		break;
	case cpu21a:
	case cpu21ya:
	case cpu21b:
	case cpu21yb:
	case cpu21s:
	case cpu21:
		board_id = CPU21;
		break;
	default: logerror("Attempt to set unknown board type %02x, defaulting to CPU20\n", board_id);
		board_id = CPU20;
	}

	return board_id | 0xbf;
}

/* ROM definitions */
ROM_START (fccpu20) /* This is an original rom dump */
	ROM_REGION32_BE(0x10000, "roms", 0)
	ROM_LOAD32_BYTE("l.bin",  0x000002, 0x4000, CRC (174ab801) SHA1 (0d7b8ed29d5fdd4bd2073005008120c5f20128dd))
	ROM_LOAD32_BYTE("ll.bin", 0x000003, 0x4000, CRC (9fd9e3e4) SHA1 (e5a7c87021e6be412dd5a8166d9f62b681169eda))
	ROM_LOAD32_BYTE("u.bin",  0x000001, 0x4000, CRC (d1afe4c0) SHA1 (b5baf9798d73632f7bb843cbc4b306c8c03f4296))
	ROM_LOAD32_BYTE("uu.bin", 0x000000, 0x4000, CRC (b54d623b) SHA1 (49b272184a04570b09004de71fae0ed0d1bf5929))
ROM_END

/* These cpu-21 boards are supported by the latest cpu-20 rom */
#define rom_fccpu21s       rom_fccpu20
#define rom_fccpu21        rom_fccpu20
#define rom_fccpu21a       rom_fccpu20
#define rom_fccpu21ya      rom_fccpu20
#define rom_fccpu21b       rom_fccpu20
#define rom_fccpu21yb      rom_fccpu20

const tiny_rom_entry *vme_fccpu20_device::device_rom_region() const
{
	LOG("%s\n", FUNCNAME);

	switch (m_board_id)
	{
	case cpu20:   return ROM_NAME( fccpu20   ); break;
	case cpu21a:  return ROM_NAME( fccpu21a  ); break;
	case cpu21ya: return ROM_NAME( fccpu21ya ); break;
	case cpu21b:  return ROM_NAME( fccpu21b  ); break;
	case cpu21yb: return ROM_NAME( fccpu21yb ); break;
	case cpu21s:  return ROM_NAME( fccpu21s  ); break;
	case cpu21:   return ROM_NAME( fccpu21   ); break;
	default: logerror("Attempt to get rom set for unknown board type %02x, defaulting to CPU20\n", m_board_id);
		return ROM_NAME( fccpu20 );
	}
}

/*
 * System ROM information
 *
 *  FORCEbug SYS68K/CPU-20 Debugging Tool Version 1.3 22-Jan-87
 *  FORCE PDOS Bootstrap , Revision 2.3 22-Jan-87
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
