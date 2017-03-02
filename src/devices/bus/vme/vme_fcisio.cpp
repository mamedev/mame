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
 * ||||--||
 * ||    ||__________________________________________________________    ___
 *       ||                                                          |_|   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |VME|
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
 * ||    ||------------------------------------------------------------+-+
 * ||||--||
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
#define TODO "Driver for 68153 BIM device needed\n"

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/scnxx562.h"
#include "machine/68230pit.h"
#include "machine/68153bim.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
#include "vme_fcisio.h"

#define LOG_GENERAL 0x01
#define LOG_SETUP   0x02
#define LOG_PRINTF  0x04

#define VERBOSE 0 //(LOG_PRINTF | LOG_SETUP  | LOG_GENERAL)

#define LOGMASK(mask, ...)   do { if (VERBOSE & mask) logerror(__VA_ARGS__); } while (0)
#define LOGLEVEL(mask, level, ...) do { if ((VERBOSE & mask) >= level) logerror(__VA_ARGS__); } while (0)

#define LOG(...)      LOGMASK(LOG_GENERAL, __VA_ARGS__)
#define LOGSETUP(...) LOGMASK(LOG_SETUP,   __VA_ARGS__)

#if VERBOSE & LOG_PRINTF
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type VME_FCISIO1 = device_creator<vme_fcisio1_card_device>;

#define CPU_CLOCK XTAL_20MHz /* HCJ */
#define DUSCC_CLOCK XTAL_14_7456MHz /* HCJ */

static ADDRESS_MAP_START (fcisio1_mem, AS_PROGRAM, 16, vme_fcisio1_card_device)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x000000, 0x000007) AM_ROM AM_READ (bootvect_r)       /* Vectors mapped from System EPROM */
	AM_RANGE (0x000000, 0x01ffff) AM_RAM /* SRAM */
	AM_RANGE (0xe00000, 0xe001ff) AM_DEVREADWRITE8("duscc0", duscc68562_device, read, write, 0x00ff)
	AM_RANGE (0xe20000, 0xe201ff) AM_DEVREADWRITE8("duscc1", duscc68562_device, read, write, 0x00ff)
	AM_RANGE (0xe40000, 0xe401ff) AM_DEVREADWRITE8("duscc2", duscc68562_device, read, write, 0x00ff)
	AM_RANGE (0xe60000, 0xe601ff) AM_DEVREADWRITE8("duscc3", duscc68562_device, read, write, 0x00ff)
	AM_RANGE (0xe80000, 0xe80dff) AM_DEVREADWRITE8("pit", pit68230_device, read, write, 0x00ff)
	AM_RANGE (0xf00000, 0xf7ffff) AM_ROM /* System EPROM Area 32Kb DEBUGGER supplied */
//  AM_RANGE (0xc40000, 0xc800ff) AM_READWRITE8 (not_implemented_r, not_implemented_w, 0xffff)  /* Dummy mapping af address area to display message */
ADDRESS_MAP_END

/*
 * Machine configuration
 */
static MACHINE_CONFIG_FRAGMENT (fcisio1)
	/* basic machine hardware */
	MCFG_CPU_ADD ("maincpu", M68010, CPU_CLOCK / 2)
	MCFG_CPU_PROGRAM_MAP (fcisio1_mem)

	MCFG_DUSCC68562_ADD("duscc0", DUSCC_CLOCK, 0, 0, 0, 0 )
	MCFG_DUSCC_OUT_TXDA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_txd))
	MCFG_DUSCC_OUT_DTRA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_dtr))
	MCFG_DUSCC_OUT_RTSA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD ("rs232trm", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("duscc0", duscc68562_device, rxa_w))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("duscc0", duscc68562_device, ctsa_w))
//  MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", fcisio_terminal)

	MCFG_DUSCC68562_ADD("duscc1", DUSCC_CLOCK, 0, 0, 0, 0 )
	MCFG_DUSCC68562_ADD("duscc2", DUSCC_CLOCK, 0, 0, 0, 0 )
	MCFG_DUSCC68562_ADD("duscc3", DUSCC_CLOCK, 0, 0, 0, 0 )

	MCFG_DEVICE_ADD ("pit", PIT68230, XTAL_20MHz / 2)
	MCFG_PIT68230_PB_INPUT_CB(READ8(vme_fcisio1_card_device, config_rd))

	MCFG_MC68153_ADD("bim", XTAL_20MHz / 2)
MACHINE_CONFIG_END

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
	ROM_LOAD16_BYTE ("ISIO-1_V2.1_L.BIN", 0xf00001, 0x4000, CRC (0d47d80f) SHA1 (541b55966f464c1cf686e36998650720950a2242))
	ROM_LOAD16_BYTE ("ISIO-1_V2.1_U.BIN", 0xf00000, 0x4000, CRC (67986768) SHA1 (215f7ff90d9dbe2bea54510e3722fb33d4e54193))
ROM_END

machine_config_constructor vme_fcisio1_card_device::device_mconfig_additions() const
{
	LOG("%s %s\n", tag(), FUNCNAME);
	return MACHINE_CONFIG_NAME( fcisio1 );
}

const tiny_rom_entry *vme_fcisio1_card_device::device_rom_region() const
{
	LOG("%s\n", FUNCNAME);
	return ROM_NAME( fcisio1 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************
vme_fcisio1_card_device::vme_fcisio1_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	,device_vme_card_interface(mconfig, *this)
	,m_maincpu (*this, "maincpu")
	,m_duscc0(*this, "duscc0")
	,m_duscc1(*this, "duscc1")
	,m_duscc2(*this, "duscc2")
	,m_duscc3(*this, "duscc3")
	,m_pit (*this, "pit")
	,m_bim (*this, "bim")
{
	LOG("%s\n", FUNCNAME);
}

vme_fcisio1_card_device::vme_fcisio1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VME_FCISIO1, "Force Computer SYS68K/ISIO-1/2 Intelligent Serial I/O Board", tag, owner, clock, "fcisio1", __FILE__)
	,device_vme_card_interface(mconfig, *this)
	,m_maincpu(*this, "maincpu")
	,m_duscc0(*this, "duscc0")
	,m_duscc1(*this, "duscc1")
	,m_duscc2(*this, "duscc2")
	,m_duscc3(*this, "duscc3")
	,m_pit (*this, "pit")
	,m_bim (*this, "bim")
{
	LOG("%s %s\n", tag, FUNCNAME);
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
