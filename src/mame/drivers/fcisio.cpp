// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  10/06/2016
 *  Force SYS68K ISIO-1/2 driver - This driver will be converted into a slot device once the VME bus driver exists.
 *  The ISIO board is a VME slave board that reads command and returns results through dual ported RAM to the VME bus.
 *
 * ISIO-1: page 385 http://bitsavers.informatik.uni-stuttgart.de/pdf/forceComputers/1988_Force_VMEbus_Products.pdf
 * ISIO-2: page 395 http://bitsavers.informatik.uni-stuttgart.de/pdf/forceComputers/1988_Force_VMEbus_Products.pdf
 *
 * Address Map - guessed/revenged no tech doc available
 * ----------------------------------------------------------
 * Address Range     Description LOCAL
 * ----------------------------------------------------------
 * 00 0000 - 00 0007 Initialisation vectors from system EPROM
 * 00 0008 - 01 FFFF Local SRAM
 * E0 0000 - E0 01FF DUSCC0
 * E2 0000 - E0 01FF DUSCC0
 * E4 0000 - E0 01FF DUSCC0
 * E6 0000 - E0 01FF DUSCC0
 * E8 0000 - E8 0DFF PI/T
 * f0 0000 - F7 0000 EPROMs
 * ----------------------------------------------------------
 * Address Range     Description LOCAL
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
#include "bus/rs232/rs232.h"
#include "machine/clock.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
#if VERBOSE == 2
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#define LLFORMAT "%I64%"
#else
#define FUNCNAME __PRETTY_FUNCTION__
#define LLFORMAT "%lld"
#endif

#define CPU_CLOCK XTAL_20MHz /* HCJ */
#define DUSCC_CLOCK XTAL_14_7456MHz /* HCJ */

class fcisio1_state : public driver_device
{
public:
fcisio1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device (mconfig, type, tag),
		m_maincpu (*this, "maincpu")
		,m_duscc0(*this, "duscc0")
		,m_duscc1(*this, "duscc1")
		,m_duscc2(*this, "duscc2")
		,m_duscc3(*this, "duscc3")
		,m_pit (*this, "pit")
{
}
	DECLARE_READ16_MEMBER (bootvect_r);
	DECLARE_READ8_MEMBER (config_rd);

	/* Dummy driver routines */
	DECLARE_READ8_MEMBER (not_implemented_r);
	DECLARE_WRITE8_MEMBER (not_implemented_w);

	virtual void machine_start () override;

protected:

private:
	required_device<cpu_device> m_maincpu;
	required_device<duscc68562_device> m_duscc0;
	required_device<duscc68562_device> m_duscc1;
	required_device<duscc68562_device> m_duscc2;
	required_device<duscc68562_device> m_duscc3;

	required_device<pit68230_device> m_pit;

	// Pointer to System ROMs needed by bootvect_r
	UINT16  *m_sysrom;
};

static ADDRESS_MAP_START (fcisio1_mem, AS_PROGRAM, 16, fcisio1_state)
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

/* Start it up */
void fcisio1_state::machine_start ()
{
	LOG (("machine_start\n"));

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (UINT16*)(memregion ("maincpu")->base () + 0xf00000);
}

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0x80000 to 0x0 */
READ16_MEMBER (fcisio1_state::bootvect_r){
	return m_sysrom [offset];
}

READ8_MEMBER (fcisio1_state::not_implemented_r){
	static int been_here = 0;
	if (!been_here++){
		logerror(TODO);
		printf(TODO);
	}
	return (UINT8) 0;
}

WRITE8_MEMBER (fcisio1_state::not_implemented_w){
	static int been_here = 0;
	if (!been_here++){
		logerror(TODO);
		printf(TODO);
	}
	return;
}

// TODO: Get a manual to understand the config options for real
READ8_MEMBER (fcisio1_state::config_rd){
	UINT8 ret = 0;
	LOG(("%s\n", FUNCNAME));

	// Port B bit #7, 0x80 Self test bit, choose either of these two lines
	ret &= ~0x80; // 0 = selftest
	//  ret |=  0x80; // 1 = no selftest

	return ret;
}

/* Input ports */
static INPUT_PORTS_START (fcisio1)
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( fcisio_terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

/*
 * Machine configuration
 */
static MACHINE_CONFIG_START (fcisio1, fcisio1_state)
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
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", fcisio_terminal)

	MCFG_DUSCC68562_ADD("duscc1", DUSCC_CLOCK, 0, 0, 0, 0 )
	MCFG_DUSCC68562_ADD("duscc2", DUSCC_CLOCK, 0, 0, 0, 0 )
	MCFG_DUSCC68562_ADD("duscc3", DUSCC_CLOCK, 0, 0, 0, 0 )

	MCFG_DEVICE_ADD ("pit", PIT68230, XTAL_20MHz / 2)
	MCFG_PIT68230_PB_INPUT_CB(READ8(fcisio1_state, config_rd))
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

/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT COMPANY                  FULLNAME          FLAGS */
COMP( 1986, fcisio1,       0,      0,      fcisio1,      fcisio1, driver_device,     0,  "Force Computers Gmbh",  "SYS68K/ISIO-1",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
