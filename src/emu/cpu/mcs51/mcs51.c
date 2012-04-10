/*****************************************************************************
 *
 *   i8051.c
 *   Portable MCS-51 Family Emulator
 *
 *   Chips in the family:
 *   8051 Product Line (8031,8051,8751)
 *   8052 Product Line (8032,8052,8752)
 *   8054 Product Line (8054)
 *   8058 Product Line (8058)
 *
 *   Copyright Steve Ellenoff, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     sellenoff@hotmail.com
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *  This work is based on:
 *  #1) 'Intel(tm) MC51 Microcontroller Family Users Manual' and
 *  #2) 8051 simulator by Travis Marlatte
 *  #3) Portable UPI-41/8041/8741/8042/8742 emulator V0.1 by Juergen Buchmueller (MAME CORE)
 *
 *****************************************************************************/

/*****************************************************************************
 *   DS5002FP emulator by Manuel Abadia
 *
 *   October 2008, couriersud: Merged back in mcs51
 *
 *   What has been added?
 *      - Extra SFRs
 *      - Bytewide Bus Support
 *      - Memory Partition and Memory Range
 *      - Bootstrap Configuration
 *      - Power Fail Interrupt
 *      - Timed Access
 *      - Stop Mode
 *      - Idle Mode
 *
 *   What is not implemented?
 *      - Peripherals and Reprogrammable Peripheral Controller
 *      - CRC-16
 *      - Watchdog timer
 *
 *   The main features of the DS5002FP are:
 *      - 100% code-compatible with 8051
 *      - Directly addresses 64kB program/64kB data memory
 *      - Nonvolatile memory control circuitry
 *      - 10-year data retention in the absence of power
 *      - In-system reprogramming via serial port
 *      - Dedicated memory bus, preserving four 8-bit ports for general purpose I/O
 *      - Power-fail reset
 *      - Early warning power-fail interrupt
 *      - Watchdog timer
 *      - Accesses up to 128kB on the bytewide bus
 *      - Decodes memory for 32kB x 8 or 128kB x 8 SRAMs
 *      - Four additional decoded peripheral-chip enables
 *      - CRC hardware for checking memory validity
 *      - Optionally emulates an 8042-style slave interface
 *      - Memory encryption using an 80-bit encryption key
 *      - Automatic random generation of encryption keys
 *      - Self-destruct input for tamper protection
 *      - Optional top-coating prevents microprobe
 *
 *****************************************************************************/

/******************************************************************************
 *  Notes:
 *
 *        The term cycles is used here to really refer to clock oscilations, because 1 machine cycle
 *        actually takes 12 oscilations.
 *
 *        Read/Write/Modify Instruction -
 *          Data is read from the Port Latch (not the Port Pin!), possibly modified, and
 *          written back to (the pin? and) the latch!
 *
 *          The following all perform this on a port address..
 *          (anl, orl, xrl, jbc, cpl, inc, dec, djnz, mov px.y,c, clr px.y, setb px.y)
 *
 *        Serial UART emulation is not really accurate, but faked enough to work as far as i can tell
 *
 *        August 27,2003: Currently support for only 8031/8051/8751 chips (ie 128 RAM)
 *        October 14,2003: Added initial support for the 8752 (ie 256 RAM)
 *        October 22,2003: Full support for the 8752 (ie 256 RAM)
 *        July 28,2004: Fixed MOVX command and added External Ram Paging Support
 *        July 31,2004: Added Serial Mode 0 Support & Fixed Interrupt Flags for Serial Port
 *
 *        October, 2008, Couriersud - Major rewrite
 *
 *****************************************************************************/

/* TODO: Varios
 *  - EA pin - defined by architecture, must implement:
 *    1 means external access, bypassing internal ROM
 *  - T0 output clock ?
 *
 * - Implement 80C52 extended serial capabilities
 * - Fix serial communication - This is a big hack (but working) right now.
 * - Implement 83C751 in sslam.c
 * - Fix cardline.c
 *      most likely due to different behaviour of I/O pins. The boards
 *      actually use 80CXX, i.e. CMOS versions.
 *      "Normal" 805X will return a 0 if reading from a output port which has
 *      a 0 written to it's latch. At least cardline expects a 1 here.
 *
 * Done: (Couriersud)
 * - Merged DS5002FP
 * - Disassembler now uses type specific memory names
 * - Merged DS5002FP disasm
 * - added 83C751 memory names to disassembler
 * - Pointer-ified
 * - Implemented cmos features
 * - Implemented 80C52 interrupt handling
 * - Fix segas18.c (segaic16.c) memory handling.
 * - Fix sslam.c
 * - Fix limenko.c videopkr.c : Issue with core allocation of ram (duplicate savestate)
 * - Handle internal ram better (debugger visible)
 *  - Fixed port reading
 *  - Rewrote Macros for better readibility
 *  - Fixed and rewrote Interrupt handling
 *  - Now returns INTERNAL_DIVIDER, adjusted cycle counts
 *  - Remove unnecessary and duplicated code
 * - Remove unnecessary functions
 * - Rewrite to have sfr-registers stored in int_ram.
 * - Debugger may now watch sfr-registers as well.
 * - implemented interrupt callbacks (HOLD_LINE now supported)
 * - Runtime switch for processor type - remove ifdefs
 * - internal memory maps for internal rom versions (internal ram now displayed in debugger)
 * - more timer cleanups from manual
 */

#include "emu.h"
#include "debugger.h"
#include "mcs51.h"

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	FEATURE_NONE			= 0x00,
	FEATURE_I8052			= 0x01,
	FEATURE_CMOS			= 0x02,
	FEATURE_I80C52			= 0x04,
	FEATURE_DS5002FP		= 0x08,
};

/* Internal address in SFR of registers */
enum
{
	ADDR_PSW	= 0xd0,
	ADDR_ACC	= 0xe0,
	ADDR_B		= 0xf0,

	ADDR_P0		= 0x80,
	ADDR_SP		= 0x81,
	ADDR_DPL	= 0x82,
	ADDR_DPH	= 0x83,
	ADDR_PCON	= 0x87,
	ADDR_TCON	= 0x88,
	ADDR_TMOD	= 0x89,
	ADDR_TL0	= 0x8a,
	ADDR_TL1	= 0x8b,
	ADDR_TH0	= 0x8c,
	ADDR_TH1	= 0x8d,
	ADDR_P1		= 0x90,
	ADDR_SCON	= 0x98,
	ADDR_SBUF	= 0x99,
	ADDR_P2		= 0xa0,
	ADDR_IE		= 0xa8,
	ADDR_P3		= 0xb0,
	ADDR_IP		= 0xb8,

	/* 8052 Only registers */
	ADDR_T2CON	= 0xc8,
	ADDR_RCAP2L	= 0xca,
	ADDR_RCAP2H	= 0xcb,
	ADDR_TL2	= 0xcc,
	ADDR_TH2	= 0xcd,

	/* 80C52 Only registers */
	ADDR_IPH	= 0xb7,
	ADDR_SADDR	= 0xa9,
	ADDR_SADEN	= 0xb9,

	/* Philips 80C52 */
	ADDR_AUXR	= 0x8e,
	ADDR_AUXR1	= 0xa2,

	/* DS5002FP */
	ADDR_CRCR	= 0xc1,
	ADDR_CRCL	= 0xc2,
	ADDR_CRCH	= 0xc3,
	ADDR_MCON	= 0xc6,
	ADDR_TA		= 0xc7,
	ADDR_RNR	= 0xcf,
	ADDR_RPCTL	= 0xd8,
	ADDR_RPS	= 0xda,

};

/* PC vectors */

enum
{
	V_RESET = 0x000,	/* power on address */
	V_IE0	= 0x003,	/* External Interrupt 0 */
	V_TF0	= 0x00b,	/* Timer 0 Overflow */
	V_IE1	= 0x013,	/* External Interrupt 1 */
	V_TF1	= 0x01b,	/* Timer 1 Overflow */
	V_RITI	= 0x023,	/* Serial Receive/Transmit */

	/* 8052 Only Vectors */
	V_TF2	= 0x02b,	/* Timer 2 Overflow */

	/* DS5002FP */
	V_PFI	= 0x02b,	/* Power Failure Interrupt */
};

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _mcs51_uart mcs51_uart;
struct _mcs51_uart
{
	UINT8	data_out;		//Data to send out
	UINT8	bits_to_send;	//How many bits left to send when transmitting out the serial port

	int		smod_div;		/* signal divided by 2^SMOD */
	int		rx_clk;			/* rx clock */
	int		tx_clk;			/* tx clock */
	UINT8	delay_cycles;	//Gross Hack;
};

typedef struct _mcs51_state_t mcs51_state_t;
struct _mcs51_state_t
{
	//Internal stuff
	UINT16	ppc;			//previous pc
	UINT16	pc;				//current pc
	UINT16	features;		//features of this cpu
	UINT8	rwm;			//Signals that the current instruction is a read/write/modify instruction

	int		inst_cycles;		/* cycles for the current instruction */
	int		ram_mask;			/* second ram bank for indirect access available ? */
	int		num_interrupts;		/* number of interrupts supported */
	int		recalc_parity;		/* recalculate parity before next instruction */
	UINT32	last_line_state;	/* last state of input lines line */
	int		t0_cnt;				/* number of 0->1 transistions on T0 line */
	int		t1_cnt;				/* number of 0->1 transistions on T1 line */
	int		t2_cnt;				/* number of 0->1 transistions on T2 line */
	int		t2ex_cnt;			/* number of 0->1 transistions on T2EX line */
	int		cur_irq_prio;		/* Holds value of the current IRQ Priority Level; -1 if no irq */
	UINT8	irq_active;			/* mask which irq levels are serviced */
	UINT8	irq_prio[8];		/* interrupt priority */

	int		icount;

	mcs51_uart uart;			/* internal uart */

	/* Internal Ram */
	UINT8	*internal_ram;		/* 128 RAM (8031/51) + 128 RAM in second bank (8032/52) */
	UINT8	*sfr_ram;			/* 128 SFR - these are in 0x80 - 0xFF */

	/* SFR Callbacks */
	void	(*sfr_write)(mcs51_state_t *mcs51_state, size_t offset, UINT8 data);
	UINT8	(*sfr_read)(mcs51_state_t *mcs51_state, size_t offset);

	/* Interrupt Callback */
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;

	/* Memory spaces */
    address_space *program;
    direct_read_data *direct;
    address_space *data;
    address_space *io;

	/* Serial Port TX/RX Callbacks */
	// TODO: Move to special port r/w
	mcs51_serial_tx_func serial_tx_callback;	//Call back funciton when sending data out of serial port
	mcs51_serial_rx_func serial_rx_callback;	//Call back function to retrieve data when receiving serial port data

	/* DS5002FP */
	struct {
		UINT8	previous_ta;		/* Previous Timed Access value */
		UINT8	ta_window;			/* Limed Access window */
		UINT8	range;				/* Memory Range */
		const ds5002fp_config *config;	/* Bootstrap Configuration */
	} ds5002fp;


};

/***************************************************************************
    MACROS
***************************************************************************/

/* Read Opcode/Opcode Arguments from Program Code */
#define ROP(pc)			mcs51_state->direct->read_decrypted_byte(pc)
#define ROP_ARG(pc)		mcs51_state->direct->read_raw_byte(pc)

/* Read a byte from External Code Memory (Usually Program Rom(s) Space) */
#define CODEMEM_R(a)	(UINT8)mcs51_state->program->read_byte(a)

/* Read/Write a byte from/to External Data Memory (Usually RAM or other I/O) */
#define DATAMEM_R(a)	(UINT8)mcs51_state->io->read_byte(a)
#define DATAMEM_W(a,v)	mcs51_state->io->write_byte(a, v)

/* Read/Write a byte from/to the Internal RAM */

#define IRAM_R(a)		iram_read(mcs51_state, a)
#define IRAM_W(a, d)	iram_write(mcs51_state, a, d)

/* Read/Write a byte from/to the Internal RAM indirectly */
/* (called from indirect addressing)                     */
INLINE UINT8 iram_iread(mcs51_state_t *mcs51_state, offs_t a) { return (a <= mcs51_state->ram_mask) ? mcs51_state->data->read_byte(a) : 0xff; }
INLINE void iram_iwrite(mcs51_state_t *mcs51_state, offs_t a, UINT8 d) { if (a <= mcs51_state->ram_mask) mcs51_state->data->write_byte(a, d); }

#define IRAM_IR(a)		iram_iread(mcs51_state, a)
#define IRAM_IW(a, d)	iram_iwrite(mcs51_state, a, d)

/* Form an Address to Read/Write to External RAM indirectly */
/* (called from indirect addressing)                        */
#define ERAM_ADDR(a,m)	external_ram_iaddr(mcs51_state, a,m)

/* Read/Write a bit from Bit Addressable Memory */
#define BIT_R(a)		bit_address_r(mcs51_state, a)
#define BIT_W(a,v)		bit_address_w(mcs51_state, a, v)

/* Input/Output a byte from given I/O port */
#define IN(port)		((UINT8)mcs51_state->io->read_byte(port))
#define OUT(port,value) mcs51_state->io->write_byte(port,value)


/***************************************************************************
    SHORTCUTS
***************************************************************************/

#define PPC 	mcs51_state->ppc
#define PC		mcs51_state->pc
#define RWM		mcs51_state->rwm

/* SFR Registers - These are accessed directly for speed on read */
/* Read accessors                                                */

#define SFR_A(a)		mcs51_state->sfr_ram[(a)]
#define SET_SFR_A(a,v)	do { SFR_A(a) = (v); } while (0)

#define ACC			((const UINT8) SFR_A(ADDR_ACC))
#define PSW			((const UINT8) SFR_A(ADDR_PSW))

#define P0			((const UINT8) SFR_A(ADDR_P0))
#define P1			((const UINT8) SFR_A(ADDR_P1))
#define P2			((const UINT8) SFR_A(ADDR_P2))
#define P3			((const UINT8) SFR_A(ADDR_P3))

#define SP			SFR_A(ADDR_SP)
#define DPL			SFR_A(ADDR_DPL)
#define DPH			SFR_A(ADDR_DPH)
#define PCON		SFR_A(ADDR_PCON)
#define TCON		SFR_A(ADDR_TCON)
#define TMOD		SFR_A(ADDR_TMOD)
#define TL0			SFR_A(ADDR_TL0)
#define TL1			SFR_A(ADDR_TL1)
#define TH0			SFR_A(ADDR_TH0)
#define TH1			SFR_A(ADDR_TH1)
#define SCON		SFR_A(ADDR_SCON)
#define IE			SFR_A(ADDR_IE)
#define IP			SFR_A(ADDR_IP)
#define B			SFR_A(ADDR_B)
#define SBUF		SFR_A(ADDR_SBUF)

#define R_REG(r)	mcs51_state->internal_ram[(r) | (PSW & 0x18)]
#define DPTR		((DPH<<8) | DPL)

/* 8052 Only registers */
#define T2CON		SFR_A(ADDR_T2CON)
#define RCAP2L		SFR_A(ADDR_RCAP2L)
#define RCAP2H		SFR_A(ADDR_RCAP2H)
#define TL2			SFR_A(ADDR_TL2)
#define TH2			SFR_A(ADDR_TH2)

/* 80C52 Only registers */
#define IPH			SFR_A(ADDR_IPH)
#define SADDR		SFR_A(ADDR_SADDR)
#define SADEN		SFR_A(ADDR_SADEN)

/* Philips 80C52 */
/* ============= */
/* Reduced EMI Mode
 * The AO bit (AUXR.0) in the AUXR register when set disables the
 * ALE output.
 */
#define AUXR		SFR_A(ADDR_AUXR)

/* The dual DPTR structure (see Figure 12) is a way by which the
 * 80C52/54/58 will specify the address of an external data memory
 * location. There are two 16-bit DPTR registers that address the
 * external memory, and a single bit called DPS = AUXR1/bit0 that
 * allows the program code to switch between them.
 */
#define AUXR1		SFR_A(ADDR_AUXR1)

/* DS5002FP only registers */
#define CRCR		SFR_A(ADDR_CRCR)
#define CRCL		SFR_A(ADDR_CRCL)
#define CRCH		SFR_A(ADDR_CRCH)
#define MCON		SFR_A(ADDR_MCON)
#define TA			SFR_A(ADDR_TA)
#define RNR			SFR_A(ADDR_RNR)
#define RPCTL		SFR_A(ADDR_RPCTL)
#define RPS			SFR_A(ADDR_RPS)


/* WRITE accessors */

/* Shortcuts */

#define SET_PSW(v)	do { SFR_A(ADDR_PSW) = (v); SET_PARITY(); } while (0)
#define SET_ACC(v)	do { SFR_A(ADDR_ACC) = (v); SET_PARITY(); } while (0)

/* These trigger actions on modification and have to be written through SFR_W */
#define SET_P0(v)	IRAM_W(ADDR_P0, v)
#define SET_P1(v)	IRAM_W(ADDR_P1, v)
#define SET_P2(v)	IRAM_W(ADDR_P2, v)
#define SET_P3(v)	IRAM_W(ADDR_P3, v)

/* Within the cpu core, do not trigger a send */
#define SET_SBUF(v)	SET_SFR_A(ADDR_SBUF, v)

/* No actions triggered on write */
#define SET_REG(r, v)	do { mcs51_state->internal_ram[(r) | (PSW & 0x18)] = (v); } while (0)

#define SET_DPTR(n)		do { DPH = ((n) >> 8) & 0xff; DPL = (n) & 0xff; } while (0)

/* Macros for Setting Flags */
#define SET_X(R, v) do { R = (v);} while (0)

#define SET_CY(n)		SET_PSW((PSW & 0x7f) | (n<<7))	//Carry Flag
#define SET_AC(n)		SET_PSW((PSW & 0xbf) | (n<<6))	//Aux.Carry Flag
#define SET_FO(n)		SET_PSW((PSW & 0xdf) | (n<<5))	//User Flag
#define SET_RS(n)		SET_PSW((PSW & 0xe7) | (n<<3))	//R Bank Select
#define SET_OV(n)		SET_PSW((PSW & 0xfb) | (n<<2))	//Overflow Flag
#define SET_P(n)		SET_PSW((PSW & 0xfe) | (n<<0))	//Parity Flag

#define SET_BIT(R, n, v) do { R = (R & ~(1<<(n))) | ((v) << (n));} while (0)
#define GET_BIT(R, n) (((R)>>(n)) & 0x01)

#define SET_EA(n)		SET_BIT(IE, 7, n)		//Global Interrupt Enable/Disable
#define SET_ES(n)		SET_BIT(IE, 4, v)		//Serial Interrupt Enable/Disable
#define SET_ET1(n)		SET_BIT(IE, 3, n)		//Timer 1 Interrupt Enable/Disable
#define SET_EX1(n)		SET_BIT(IE, 2, n)		//External Int 1 Interrupt Enable/Disable
#define SET_ET0(n)		SET_BIT(IE, 1, n)		//Timer 0 Interrupt Enable/Disable
#define SET_EX0(n)		SET_BIT(IE, 0, n)		//External Int 0 Interrupt Enable/Disable
/* 8052 Only flags */
#define SET_ET2(n)		SET_BIT(IE, 5, n)		//Timer 2 Interrupt Enable/Disable

/* 8052 Only flags */
#define SET_PT2(n)		SET_BIT(IP, 5, n);	//Set Timer 2 Priority Level

#define SET_PS0(n)		SET_BIT(IP, 4, n)		//Set Serial Priority Level
#define SET_PT1(n)		SET_BIT(IP, 3, n)		//Set Timer 1 Priority Level
#define SET_PX1(n)		SET_BIT(IP, 2, n)		//Set External Int 1 Priority Level
#define SET_PT0(n)		SET_BIT(IP, 1, n)		//Set Timer 0 Priority Level
#define SET_PX0(n)		SET_BIT(IP, 0, n)		//Set External Int 0 Priority Level

#define SET_TF1(n)		SET_BIT(TCON, 7, n)	//Indicated Timer 1 Overflow Int Triggered
#define SET_TR1(n)		SET_BIT(TCON, 6, n)  //IndicateS Timer 1 is running
#define SET_TF0(n)		SET_BIT(TCON, 5, n)	//Indicated Timer 0 Overflow Int Triggered
#define SET_TR0(n)		SET_BIT(TCON, 4, n)  //IndicateS Timer 0 is running
#define SET_IE1(n)		SET_BIT(TCON, 3, n)  //Indicated External Int 1 Triggered
#define SET_IT1(n)		SET_BIT(TCON, 2, n)  //Indicates how External Int 1 is Triggered
#define SET_IE0(n)		SET_BIT(TCON, 1, n)  //Indicated External Int 0 Triggered
#define SET_IT0(n)		SET_BIT(TCON, 0, n)  //Indicates how External Int 0 is Triggered

#define SET_SM0(n)		SET_BIT(SCON, 7, n)	//Sets Serial Port Mode
#define SET_SM1(n)		SET_BIT(SCON, 6, n)  //Sets Serial Port Mode
#define SET_SM2(n)		SET_BIT(SCON, 5, n)	//Sets Serial Port Mode (Multiprocesser mode)
#define SET_REN(n)		SET_BIT(SCON, 4, n)  //Sets Serial Port Receive Enable
#define SET_TB8(n)		SET_BIT(SCON, 3, n)  //Transmit 8th Bit
#define SET_RB8(n)		SET_BIT(SCON, 2, n)  //Receive 8th Bit
#define SET_TI(n)		SET_BIT(SCON, 1, n)  //Indicates Transmit Interrupt Occurred
#define SET_RI(n)		SET_BIT(SCON, 0, n)  //Indicates Receive Interrupt Occurred

#define SET_GATE1(n)	SET_BIT(TMOD, 7, n)	//Timer 1 Gate Mode
#define SET_CT1(n)		SET_BIT(TMOD, 6, n)  //Timer 1 Counter Mode
#define SET_M1_1(n)		SET_BIT(TMOD, 5, n)	//Timer 1 Timer Mode Bit 1
#define SET_M1_0(n)		SET_BIT(TMOD, 4, n)  //Timer 1 Timer Mode Bit 0
#define SET_GATE0(n)	SET_BIT(TMOD, 3, n)  //Timer 0 Gate Mode
#define SET_CT0(n)		SET_BIT(TMOD, 2, n)  //Timer 0 Counter Mode
#define SET_M0_1(n)		SET_BIT(TMOD, 1, n)  //Timer 0 Timer Mode Bit 1
#define SET_M0_0(n)		SET_BIT(TMOD, 0, n)  //Timer 0 Timer Mode Bit 0



/* 8052 Only flags - T2CON Flags */
#define SET_TF2(n)		SET_BIT(T2CON, 7, n)	//Indicated Timer 2 Overflow Int Triggered
#define SET_EXF2(n)		SET_BIT(T2CON, 6, n)	//Indicates Timer 2 External Flag
#define SET_RCLK(n)		SET_BIT(T2CON, 5, n)	//Receive Clock
#define SET_TCLK(n)		SET_BIT(T2CON, 4, n)	//Transmit Clock
#define SET_EXEN2(n)	SET_BIT(T2CON, 3, n)	//Timer 2 External Interrupt Enable
#define SET_TR2(n)		SET_BIT(T2CON, 2, n)	//Indicates Timer 2 is running
#define SET_CT2(n)		SET_BIT(T2CON, 1, n)	//Sets Timer 2 Counter/Timer Mode
#define SET_CP(n)		SET_BIT(T2CON, 0, n)	//Sets Timer 2 Capture/Reload Mode

#define SET_GF1(n)		SET_BIT(PCON, 3, n)
#define SET_GF0(n)		SET_BIT(PCON, 2, n)
#define SET_PD(n)		SET_BIT(PCON, 1, n)
#define SET_IDL(n)		SET_BIT(PCON, 0, n)

/* Macros for accessing flags */

#define GET_CY			GET_BIT(PSW, 7)
#define GET_AC			GET_BIT(PSW, 6)
#define GET_FO			GET_BIT(PSW, 5)
#define GET_RS			GET_BIT(PSW, 3)
#define GET_OV			GET_BIT(PSW, 2)
#define GET_P			GET_BIT(PSW, 0)

#define GET_EA			GET_BIT(IE, 7)
#define GET_ET2			GET_BIT(IE, 5)
#define GET_ES			GET_BIT(IE, 4)
#define GET_ET1			GET_BIT(IE, 3)
#define GET_EX1			GET_BIT(IE, 2)
#define GET_ET0			GET_BIT(IE, 1)
#define GET_EX0			GET_BIT(IE, 0)

/* 8052 Only flags */
#define GET_PT2			GET_BIT(IP, 5)

#define GET_PS			GET_BIT(IP, 4)
#define GET_PT1			GET_BIT(IP, 3)
#define GET_PX1			GET_BIT(IP, 2)
#define GET_PT0			GET_BIT(IP, 1)
#define GET_PX0			GET_BIT(IP, 0)

#define GET_TF1			GET_BIT(TCON, 7)
#define GET_TR1			GET_BIT(TCON, 6)
#define GET_TF0			GET_BIT(TCON, 5)
#define GET_TR0			GET_BIT(TCON, 4)
#define GET_IE1			GET_BIT(TCON, 3)
#define GET_IT1			GET_BIT(TCON, 2)
#define GET_IE0			GET_BIT(TCON, 1)
#define GET_IT0			GET_BIT(TCON, 0)

#define GET_SM0			GET_BIT(SCON, 7)
#define GET_SM1			GET_BIT(SCON, 6)
#define GET_SM2			GET_BIT(SCON, 5)
#define GET_REN			GET_BIT(SCON, 4)
#define GET_TB8			GET_BIT(SCON, 3)
#define GET_RB8			GET_BIT(SCON, 2)
#define GET_TI			GET_BIT(SCON, 1)
#define GET_RI			GET_BIT(SCON, 0)

#define GET_GATE1		GET_BIT(TMOD, 7)
#define GET_CT1			GET_BIT(TMOD, 6)
#define GET_M1_1		GET_BIT(TMOD, 5)
#define GET_M1_0		GET_BIT(TMOD, 4)
#define GET_GATE0		GET_BIT(TMOD, 3)
#define GET_CT0			GET_BIT(TMOD, 2)
#define GET_M0_1		GET_BIT(TMOD, 1)
#define GET_M0_0		GET_BIT(TMOD, 0)

#define GET_SMOD		GET_BIT(PCON, 7)

/* Only in 80C51BH & other cmos */

#define GET_GF1			GET_BIT(PCON, 3)
#define GET_GF0			GET_BIT(PCON, 2)
#define GET_PD			GET_BIT(PCON, 1)
#define GET_IDL			(GET_BIT(PCON, 0) & ~(GET_PD))	/* PD takes precedence! */

/* 8052 Only flags */
#define GET_TF2			GET_BIT(T2CON, 7)
#define GET_EXF2		GET_BIT(T2CON, 6)
#define GET_RCLK		GET_BIT(T2CON, 5)
#define GET_TCLK		GET_BIT(T2CON, 4)
#define GET_EXEN2		GET_BIT(T2CON, 3)
#define GET_TR2			GET_BIT(T2CON, 2)
#define GET_CT2			GET_BIT(T2CON, 1)
#define GET_CP			GET_BIT(T2CON, 0)

/* DS5002FP Only flags */

/* PCON Flags - DS5002FP */

#define GET_POR			GET_BIT(PCON, 6)
#define GET_PFW			GET_BIT(PCON, 5)
#define GET_WTR			GET_BIT(PCON, 4)
#define GET_EPFW		GET_BIT(PCON, 3)
#define GET_EWT			GET_BIT(PCON, 2)

#define	SET_PFW(n)		SET_BIT(PCON, 5, n)

/* MCON Flags - DS5002FP */

#define GET_PA			((MCON & 0xf0)>>4)
#define GET_RG1			GET_BIT(MCON, 3)
#define GET_PES			GET_BIT(MCON, 2)
#define GET_PM			GET_BIT(MCON, 1)
#define GET_SL			GET_BIT(MCON, 0)

/* RPCTL Flags - DS5002FP */
#define GET_RNR			GET_BIT(RPCTL, 7) /* Bit 6 ?? */
#define GET_EXBS		GET_BIT(RPCTL, 5)
#define GET_AE			GET_BIT(RPCTL, 4)
#define GET_IBI			GET_BIT(RPCTL, 3)
#define GET_DMA			GET_BIT(RPCTL, 2)
#define GET_RPCON		GET_BIT(RPCTL, 1)
#define GET_RG0			GET_BIT(RPCTL, 0)


/*Add and Subtract Flag settings*/
#define DO_ADD_FLAGS(a,d,c)	do_add_flags(mcs51_state, a, d, c)
#define DO_SUB_FLAGS(a,d,c)	do_sub_flags(mcs51_state, a, d, c)

#define SET_PARITY()	do {mcs51_state->recalc_parity |= 1;} while (0)
#define PUSH_PC()		push_pc(mcs51_state)
#define POP_PC()		pop_pc(mcs51_state)

/* Clear Current IRQ  */
#define CLEAR_CURRENT_IRQ() clear_current_irq(mcs51_state)

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void check_irqs(mcs51_state_t *mcs51_state);
INLINE void serial_transmit(mcs51_state_t *mcs51_state, UINT8 data);

/* Hold callback functions so they can be set by caller (before the cpu reset) */

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE mcs51_state_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == I8031 ||
		   device->type() == I8032 ||
		   device->type() == I8051 ||
		   device->type() == I8751 ||
		   device->type() == I8052 ||
		   device->type() == I8752 ||
		   device->type() == I80C31 ||
		   device->type() == I80C32 ||
		   device->type() == I80C51 ||
		   device->type() == I80C52 ||
		   device->type() == I87C51 ||
		   device->type() == I87C52 ||
		   device->type() == AT89C4051 ||
		   device->type() == DS5002FP);
	return (mcs51_state_t *)downcast<legacy_cpu_device *>(device)->token();
}

INLINE void clear_current_irq(mcs51_state_t *mcs51_state)
{
	if (mcs51_state->cur_irq_prio >= 0)
		mcs51_state->irq_active &= ~(1 << mcs51_state->cur_irq_prio);
	if (mcs51_state->irq_active & 4)
		mcs51_state->cur_irq_prio = 2;
	else if (mcs51_state->irq_active & 2)
		mcs51_state->cur_irq_prio = 1;
	else if (mcs51_state->irq_active & 1)
		mcs51_state->cur_irq_prio = 0;
	else
		mcs51_state->cur_irq_prio = -1;
	LOG(("New: %d %02x\n", mcs51_state->cur_irq_prio, mcs51_state->irq_active));
}

INLINE UINT8 r_acc(mcs51_state_t *mcs51_state) { return SFR_A(ADDR_ACC); }

INLINE UINT8 r_psw(mcs51_state_t *mcs51_state) { return SFR_A(ADDR_PSW); }

INLINE void update_ptrs(mcs51_state_t *mcs51_state)
{
	mcs51_state->internal_ram = (UINT8 *)mcs51_state->data->get_write_ptr(0x00);
	mcs51_state->sfr_ram = (UINT8 *)mcs51_state->data->get_write_ptr(0x100);
}


/* Generate an external ram address for read/writing using indirect addressing mode */

/*The lowest 8 bits of the address are passed in (from the R0/R1 register), however
  the hardware can be configured to set the rest of the address lines to any available output port pins, which
  means the only way we can implement this is to allow the driver to setup a callback to generate the
  address as defined by the specific hardware setup. We'll assume the address won't be bigger than 32 bits

  Couriersud, October 2008:
  There is no way external hardware can distinguish between 8bit access and 16 bit access.
  During 16bit access the high order byte of the address is output on port 2. We therefore
  assume that most hardware will use port 2 for 8bit access as well.

  On configurations where 8 bit access in conjunction with other ports is used,
  it is up to the driver to use AM_MIRROR to mask out the high level address and
  provide it's own mapping.
*/

/*
    The DS5002FP has 2 16 bits data address buses (the byte-wide bus and the expanded bus). The exact memory position accessed depends on the
    partition mode, the memory range and the expanded bus select. The partition mode and the expanded bus select can be changed at any time.

    In order to simplify memory mapping to the data address bus, the following address map is assumed for partitioned mode:

    0x00000-0x0ffff -> data memory on the expanded bus
    0x10000-0x1ffff -> data memory on the byte-wide bus

    For non-partitioned mode the following memory map is assumed:

    0x0000-0xffff -> data memory (the bus used to access it does not matter)
*/

INLINE offs_t external_ram_iaddr(mcs51_state_t *mcs51_state, offs_t offset, offs_t mem_mask)
{
	/* Memory Range (RG1 and RG0 @ MCON and RPCTL registers) */
	static const UINT16 ds5002fp_ranges[4] = { 0x1fff, 0x3fff, 0x7fff, 0xffff };
	/* Memory Partition Table (RG1 & RG0 @ MCON & RPCTL registers) */
	static const UINT32 ds5002fp_partitions[16] = {
		0x0000, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000,  0x7000,
		0x8000, 0x9000, 0xa000, 0xb000, 0xc000, 0xd000, 0xe000,	0x10000 };

	/* if partition mode is set, adjust offset based on the bus */
	if (mcs51_state->features & FEATURE_DS5002FP)
	{
		if (!GET_PM) {
			if (!GET_EXBS) {
				if ((offset >= ds5002fp_partitions[GET_PA]) && (offset <= ds5002fp_ranges[mcs51_state->ds5002fp.range])) {
					offset += 0x10000;
				}
			}
		}
	}
	else
	{
		if (mem_mask == 0x00ff)
			return (offset & mem_mask) | (P2 << 8);
	}
	return offset;
}

/* Internal ram read/write */

INLINE UINT8 iram_read(mcs51_state_t *mcs51_state, size_t offset)
{
	return (((offset) < 0x80) ? mcs51_state->data->read_byte(offset) : mcs51_state->sfr_read(mcs51_state, offset));
}

INLINE void iram_write(mcs51_state_t *mcs51_state, size_t offset, UINT8 data)
{
	if ((offset) < 0x80)
		mcs51_state->data->write_byte(offset, data);
	else
		mcs51_state->sfr_write(mcs51_state, offset, data);
}

/*Push the current PC to the stack*/
INLINE void push_pc(mcs51_state_t *mcs51_state)
{
	UINT8 tmpSP = SP+1;						//Grab and Increment Stack Pointer
	IRAM_IW(tmpSP, (PC & 0xff));				//Store low byte of PC to Internal Ram (Use IRAM_IW to store stack above 128 bytes)
	tmpSP++;									// ""
	SP = tmpSP;								// ""
	IRAM_IW(tmpSP, ( (PC & 0xff00) >> 8));		//Store hi byte of PC to next address in Internal Ram (Use IRAM_IW to store stack above 128 bytes)
}

/*Pop the current PC off the stack and into the pc*/
INLINE void pop_pc(mcs51_state_t *mcs51_state)
{
	UINT8 tmpSP = SP;							//Grab Stack Pointer
	PC = (IRAM_IR(tmpSP--) & 0xff) << 8;		//Store hi byte to PC (must use IRAM_IR to access stack pointing above 128 bytes)
	PC = PC | IRAM_IR(tmpSP--);					//Store lo byte to PC (must use IRAM_IR to access stack pointing above 128 bytes)
	SP = tmpSP;								//Decrement Stack Pointer
}

//Set the PSW Parity Flag
INLINE void set_parity(mcs51_state_t *mcs51_state)
{
	//This flag will be set when the accumulator contains an odd # of bits set..
	UINT8 p = 0;
	int i;
	UINT8 a = ACC;

	for (i=0; i<8; i++) {		//Test for each of the 8 bits in the ACC!
		p ^= (a & 1);
		a = (a >> 1);
	}

	SET_P(p & 1);
}

INLINE UINT8 bit_address_r(mcs51_state_t *mcs51_state, UINT8 offset)
{
	UINT8	word;
	UINT8	mask;
	int	bit_pos;
	int	distance;	/* distance between bit addressable words */
					/* 1 for normal bits, 8 for sfr bit addresses */

	//User defined bit addresses 0x20-0x2f (values are 0x0-0x7f)
	if (offset < 0x80) {
		distance = 1;
		word = ( (offset & 0x78) >> 3) * distance + 0x20;
		bit_pos = offset & 0x7;
		mask = (0x1 << bit_pos);
		return((IRAM_R(word) & mask) >> bit_pos);
	}
	//SFR bit addressable registers
	else {
		distance = 8;
		word = ( (offset & 0x78) >> 3) * distance + 0x80;
		bit_pos = offset & 0x7;
		mask = (0x1 << bit_pos);
		return ((IRAM_R(word) & mask) >> bit_pos);
	}
}


INLINE void bit_address_w(mcs51_state_t *mcs51_state, UINT8 offset, UINT8 bit)
{
	int	word;
	UINT8	mask;
	int	bit_pos;
	UINT8	result;
	int	distance;

	/* User defined bit addresses 0x20-0x2f (values are 0x0-0x7f) */
	if (offset < 0x80) {
		distance = 1;
		word = ((offset & 0x78) >> 3) * distance + 0x20;
		bit_pos = offset & 0x7;
		bit = (bit & 0x1) << bit_pos;
		mask = ~(1 << bit_pos) & 0xff;
		result = IRAM_R(word) & mask;
		result = result | bit;
		IRAM_W(word, result);
	}
	/* SFR bit addressable registers */
	else {
		distance = 8;
		word = ((offset & 0x78) >> 3) * distance + 0x80;
		bit_pos = offset & 0x7;
		bit = (bit & 0x1) << bit_pos;
		mask = ~(1 << bit_pos) & 0xff;
		result = IRAM_R(word) & mask;
		result = result | bit;
		IRAM_W(word, result);
	}
}

INLINE void do_add_flags(mcs51_state_t *mcs51_state, UINT8 a, UINT8 data, UINT8 c)
{
	UINT16 result = a+data+c;
	INT16 result1 = (INT8)a+(INT8)data+c;

	SET_CY((result & 0x100) >> 8);
	result = (a&0x0f)+(data&0x0f)+c;
	SET_AC((result & 0x10) >> 4);
	SET_OV(result1 < -128 || result1 > 127);
}

INLINE void do_sub_flags(mcs51_state_t *mcs51_state, UINT8 a, UINT8 data, UINT8 c)
{
	UINT16 result = a-(data+c);
	INT16 result1 = (INT8)a-(INT8)(data+c);

	SET_CY((result & 0x100) >> 8);
	result = (a&0x0f)-((data&0x0f)+c);
	SET_AC((result & 0x10) >> 4);
	SET_OV((result1 < -128 || result1 > 127));
}

INLINE void transmit_receive(mcs51_state_t *mcs51_state, int source)
{
	int mode = (GET_SM0<<1) | GET_SM1;

	if (source == 1) /* timer1 */
		mcs51_state->uart.smod_div = (mcs51_state->uart.smod_div + 1) & (2-GET_SMOD);

	switch(mode) {
		//8 bit shifter ( + start,stop bit ) - baud set by clock freq / 12
		case 0:
			mcs51_state->uart.rx_clk += (source == 0) ? 16 : 0; /* clock / 12 */
			mcs51_state->uart.tx_clk += (source == 0) ? 16 : 0; /* clock / 12 */
			break;
		//8 bit uart ( + start,stop bit ) - baud set by timer1 or timer2
		case 1:
		case 3:
			if (source == 1)
			{
				mcs51_state->uart.tx_clk += (GET_TCLK ? 0 : !mcs51_state->uart.smod_div);
				mcs51_state->uart.rx_clk += (GET_RCLK ? 0 : !mcs51_state->uart.smod_div);
			}
			if (source == 2)
			{
				mcs51_state->uart.tx_clk += (GET_TCLK ? 1 : 0);
				mcs51_state->uart.rx_clk += (GET_RCLK ? 1 : 0);
			}
			break;
		//9 bit uart
		case 2:
			mcs51_state->uart.rx_clk += (source == 0) ? (GET_SMOD ? 6 : 3) : 0; /* clock / 12 * 3 / 8 (16) = clock / 32 (64)*/
			mcs51_state->uart.tx_clk += (source == 0) ? (GET_SMOD ? 6 : 3) : 0; /* clock / 12 */
			break;
	}
	/* transmit ? */
	if (mcs51_state->uart.tx_clk >= 16)
	{
		mcs51_state->uart.tx_clk &= 0x0f;
		if(mcs51_state->uart.bits_to_send)
		{
			mcs51_state->uart.bits_to_send--;
			if(mcs51_state->uart.bits_to_send == 0) {
				//Call the callback function
				if(mcs51_state->serial_tx_callback)
					mcs51_state->serial_tx_callback(mcs51_state->device, mcs51_state->uart.data_out);
				//Set Interrupt Flag
				SET_TI(1);
			}
		}

	}
	/* receive */
	if (mcs51_state->uart.rx_clk >= 16)
	{
		mcs51_state->uart.rx_clk &= 0x0f;
		if (mcs51_state->uart.delay_cycles>0)
		{
			mcs51_state->uart.delay_cycles--;
			if (mcs51_state->uart.delay_cycles == 0)
			{
				int data = 0;
				//Call our callball function to retrieve the data
				if(mcs51_state->serial_rx_callback)
					data = mcs51_state->serial_rx_callback(mcs51_state->device);
				LOG(("RX Deliver %d\n", data));
				SET_SBUF(data);
				//Flag the IRQ
				SET_RI(1);
			}
		}
	}
}


INLINE void update_timer_t0(mcs51_state_t *mcs51_state, int cycles)
{
	int mode = (GET_M0_1<<1) | GET_M0_0;
	UINT32 count = 0;

	if (GET_TR0)
	{
		UINT32 delta;

		/* counter / external input */
		delta = GET_CT0 ? mcs51_state->t0_cnt : cycles;
		/* taken, reset */
		mcs51_state->t0_cnt = 0;
		/* TODO: Not sure about IE0. The manual specifies INT0=high,
         * which in turn means CLEAR_LINE.
         * IE0 may be edge triggered depending on IT0 */
		if (GET_GATE0 && !GET_IE0)
			delta = 0;

		switch(mode) {
			case 0:			/* 13 Bit Timer Mode */
				count = ((TH0<<5) | ( TL0 & 0x1f ) );
				count += delta;
				if ( count & 0xffffe000 ) /* Check for overflow */
					SET_TF0(1);
				TH0 = (count>>5) & 0xff;
				TL0 =  count & 0x1f ;
				break;
			case 1:			/* 16 Bit Timer Mode */
				count = ((TH0<<8) | TL0);
				count += delta;
				if ( count & 0xffff0000 ) /* Check for overflow */
					SET_TF0(1);
				TH0 = (count>>8) & 0xff;
				TL0 = count & 0xff;
				break;
			case 2:			/* 8 Bit Autoreload */
				count = ((UINT32) TL0) + delta;
				if ( count & 0xffffff00 )				/* Check for overflow */
				{
					SET_TF0(1);
					count += TH0;						/* Reload timer */
				}
				/* Update new values of the counter */
				TL0 =  count & 0xff;
				break;
			case 3:
				/* Split Timer 1 */
				count = ((UINT32) TL0) + delta;
				if ( count & 0xffffff00 )				/* Check for overflow */
					SET_TF0(1);
				TL0 = count & 0xff; 					/* Update new values of the counter */
				break;
		}
	}
	if (GET_TR1)
	{
		switch(mode)
		{
		case 3:
			/* Split Timer 2 */
			count = ((UINT32) TH0) + cycles;			/* No gate control or counting !*/
			if ( count & 0xffffff00 )				/* Check for overflow */
				SET_TF1(1);
			TH0 = count & 0xff;						/* Update new values of the counter */
			break;
		}
	}
}

/* From the DS5002FP User Manual
When Timer 1 is selected for operation in Mode 3, it stops counting and holds its current value. This
action is the same as setting TR1 = 0. When Timer 0 is selected in Mode 3, Timer 1???s control bits are
stolen as described above. As a result, Timer 1???s functions are limited in this MODE. It is forced to
operate as a timer whose clock in-put is 12 tCLK and it cannot generate an interrupt on overflow. In
addition, it also cannot be used with the GATE function. However, it can be started and stopped by
switching it into or out of Mode 3 or it can be assigned as a baud rate generator for the serial port.
*/

/* Intel documentation:
 *  Timer 1 may still be used in modes 0, 1, and 2, while timer 0
 * is in mode 3. With one important exception:  No interrupts
 * will be generated by timer 1 while timer 0 is using the TF1
 * overflow flag
 */

INLINE void update_timer_t1(mcs51_state_t *mcs51_state, int cycles)
{
	UINT8 mode = (GET_M1_1<<1) | GET_M1_0;
	UINT8 mode_0 = (GET_M0_1<<1) | GET_M0_0;
	UINT32 count = 0;

	if (mode_0 != 3)
	{
		if (GET_TR1)
		{
			UINT32 delta;
			UINT32 overflow = 0;

			/* counter / external input */
			delta = GET_CT1 ? mcs51_state->t1_cnt : cycles;
			/* taken, reset */
			mcs51_state->t1_cnt = 0;
			/* TODO: Not sure about IE0. The manual specifies INT0=high,
             * which in turn means CLEAR_LINE. Change to access last_state?
             * IE0 may be edge triggered depending on IT0 */
			if (GET_GATE1 && !GET_IE1)
				delta = 0;

			switch(mode) {
				case 0:			/* 13 Bit Timer Mode */
					count = ((TH1<<5) | ( TL1 & 0x1f ) );
					count += delta;
					overflow = count & 0xffffe000; /* Check for overflow */
					TH1 = (count>>5) & 0xff;
					TL1 =  count & 0x1f ;
					break;
				case 1:			/* 16 Bit Timer Mode */
					count = ((TH1<<8) | TL1);
					count += delta;
					overflow = count & 0xffff0000; /* Check for overflow */
					TH1 = (count>>8) & 0xff;
					TL1 = count & 0xff;
					break;
				case 2:			/* 8 Bit Autoreload */
					count = ((UINT32) TL1) + delta;
					overflow = count & 0xffffff00; /* Check for overflow */
					if ( overflow )
					{
						count += TH1;						/* Reload timer */
					}
					/* Update new values of the counter */
					TL1 =  count & 0xff;
					break;
				case 3:
					/* do nothing */
					break;
			}
			if (overflow)
			{
				SET_TF1(1);
				transmit_receive(mcs51_state, 1);
			}
		}
	}
	else
	{
		UINT32 delta;
		UINT32 overflow = 0;

		delta =  cycles;
		/* taken, reset */
		mcs51_state->t1_cnt = 0;
		switch(mode) {
			case 0:			/* 13 Bit Timer Mode */
				count = ((TH1<<5) | ( TL1 & 0x1f ) );
				count += delta;
				overflow = count & 0xffffe000; /* Check for overflow */
				TH1 = (count>>5) & 0xff;
				TL1 =  count & 0x1f ;
				break;
			case 1:			/* 16 Bit Timer Mode */
				count = ((TH1<<8) | TL1);
				count += delta;
				overflow = count & 0xffff0000; /* Check for overflow */
				TH1 = (count>>8) & 0xff;
				TL1 = count & 0xff;
				break;
			case 2:			/* 8 Bit Autoreload */
				count = ((UINT32) TL1) + delta;
				overflow = count & 0xffffff00; /* Check for overflow */
				if ( overflow )
				{
					count += TH1;						/* Reload timer */
				}
				/* Update new values of the counter */
				TL1 =  count & 0xff;
				break;
			case 3:
				/* do nothing */
				break;
		}
		if (overflow)
		{
			transmit_receive(mcs51_state, 1);
		}
	}
}

INLINE void update_timer_t2(mcs51_state_t *mcs51_state, int cycles)
{
	/* Update Timer 2 */
	if(GET_TR2) {
		int mode = ((GET_TCLK | GET_RCLK) << 1) | GET_CP;
		int delta = GET_CT2 ? mcs51_state->t2_cnt : (mode & 2) ? cycles * (12/2) : cycles;

		UINT32 count = ((TH2<<8) | TL2) + delta;
		mcs51_state->t2_cnt = 0;

		switch (mode)
		{
			case 0:	/* 16 Bit Auto Reload */
				if ( count & 0xffff0000 )
				{
					SET_TF2(1);
					count += ((RCAP2H<<8) | RCAP2L);
				}
				else if (GET_EXEN2 && mcs51_state->t2ex_cnt>0)
				{
					count += ((RCAP2H<<8) | RCAP2L);
					mcs51_state->t2ex_cnt = 0;
				}
				TH2 = (count>>8) & 0xff;
				TL2 =  count & 0xff;
				break;
			case 1: /* 16 Bit Capture */
				if ( count & 0xffff0000 )
					SET_TF2(1);
				TH2 = (count>>8) & 0xff;
				TL2 =  count & 0xff;

				if (GET_EXEN2 && mcs51_state->t2ex_cnt>0)
				{
					RCAP2H = TH2;
					RCAP2L = TL2;
					mcs51_state->t2ex_cnt = 0;
				}
				break;
			case 2:
			case 3:	/* Baud rate */
				if ( count & 0xffff0000 )
				{
					count += ((RCAP2H<<8) | RCAP2L);
					transmit_receive(mcs51_state, 2);
				}
				TH2 = (count>>8) & 0xff;
				TL2 =  count & 0xff;
				break;
		}
	}
}

INLINE void update_timers(mcs51_state_t *mcs51_state, int cycles)
{
	/* Update Timer 0 */
	update_timer_t0(mcs51_state, cycles);
	update_timer_t1(mcs51_state, cycles);

	if (mcs51_state->features & FEATURE_I8052)
	{
		update_timer_t2(mcs51_state, cycles);
	}
}

//Set up to transmit data out of serial port
//NOTE: Enable Serial Port Interrupt bit is NOT required to send/receive data!

INLINE void serial_transmit(mcs51_state_t *mcs51_state, UINT8 data)
{
	int mode = (GET_SM0<<1) | GET_SM1;

	//Flag that we're sending data
	mcs51_state->uart.data_out = data;
	LOG(("serial_tansmit: %x %x\n", mode, data));
	switch(mode) {
		//8 bit shifter ( + start,stop bit ) - baud set by clock freq / 12
		case 0:
			mcs51_state->uart.bits_to_send = 8+2;
			break;
		//8 bit uart ( + start,stop bit ) - baud set by timer1 or timer2
		case 1:
			mcs51_state->uart.bits_to_send = 8+2;
			break;
		//9 bit uart
		case 2:
		case 3:
			LOG(("Serial mode %d not supported in mcs51!\n", mode));
			break;
	}
}

INLINE void serial_receive(mcs51_state_t *mcs51_state)
{
	int mode = (GET_SM0<<1) | GET_SM1;

	if (GET_REN) {
		switch(mode) {
			//8 bit shifter ( + start,stop bit ) - baud set by clock freq / 12
			case 0:
				mcs51_state->uart.delay_cycles = 8+2;
				break;
			//8 bit uart ( + start,stop bit ) - baud set by timer1 or timer2
			case 1:
				mcs51_state->uart.delay_cycles = 8+2;
				break;
			//9 bit uart
			case 2:
			case 3:
				LOG(("Serial mode %d not supported in mcs51!\n", mode));
				break;
		}
	}
}

/* Check and update status of serial port */
INLINE void	update_serial(mcs51_state_t *mcs51_state, int cycles)
{
	while (--cycles>=0)
		transmit_receive(mcs51_state, 0);
}

/* Check and update status of serial port */
INLINE void	update_irq_prio(mcs51_state_t *mcs51_state, UINT8 ipl, UINT8 iph)
{
	int i;
	for (i=0; i<8; i++)
		mcs51_state->irq_prio[i] = ((ipl >> i) & 1) | (((iph >>i ) & 1) << 1);
}

/***************************************************************************
    CALLBACKS - TODO: Remove
***************************************************************************/


void i8051_set_serial_tx_callback(device_t *device, mcs51_serial_tx_func tx_func)
{
	mcs51_state_t *mcs51_state = get_safe_token(device);
	mcs51_state->serial_tx_callback = tx_func;
}

void i8051_set_serial_rx_callback(device_t *device, mcs51_serial_rx_func rx_func)
{
	mcs51_state_t *mcs51_state = get_safe_token(device);
	mcs51_state->serial_rx_callback = rx_func;
}

/***************************************************************************
    OPCODES
***************************************************************************/

#define OPHANDLER( _name ) INLINE void _name (mcs51_state_t *mcs51_state, UINT8 r)

#include "mcs51ops.c"


static void execute_op(mcs51_state_t *mcs51_state, UINT8 op)
{
	if (mcs51_state->recalc_parity)
	{
		set_parity(mcs51_state);
		mcs51_state->recalc_parity = 0;
	}

	switch( op )
	{

		case 0x00:	nop(mcs51_state, op);							break;	//NOP
		case 0x01:	ajmp(mcs51_state, op);						break;	//AJMP code addr
		case 0x02:	ljmp(mcs51_state, op);						break;	//LJMP code addr
		case 0x03:	rr_a(mcs51_state, op);						break;	//RR A
		case 0x04:	inc_a(mcs51_state, op);						break;	//INC A
		case 0x05:	RWM=1; inc_mem(mcs51_state, op); RWM=0;		break;	//INC data addr

		case 0x06:
		case 0x07:	inc_ir(mcs51_state,op&1);						break;	//INC @R0/@R1

		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:	inc_r(mcs51_state, op&7);						break;	//INC R0 to R7

		case 0x10:	RWM=1; jbc(mcs51_state, op); RWM=0; 			break;	//JBC bit addr, code addr
		case 0x11:	acall(mcs51_state, op);						break;	//ACALL code addr
		case 0x12:	lcall(mcs51_state, op); 						break;	//LCALL code addr
		case 0x13:	rrc_a(mcs51_state, op);						break;	//RRC A
		case 0x14:	dec_a(mcs51_state, op);						break;	//DEC A
		case 0x15:	RWM=1; dec_mem(mcs51_state, op); RWM=0;		break;	//DEC data addr

		case 0x16:
		case 0x17:	dec_ir(mcs51_state, op&1);					break;	//DEC @R0/@R1

		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:	dec_r(mcs51_state, op&7);					break;	//DEC R0 to R7

		case 0x20:	jb(mcs51_state, op);							break;	//JB  bit addr, code addr
		case 0x21:	ajmp(mcs51_state, op);						break;	//AJMP code addr
		case 0x22:	ret(mcs51_state, op);							break;	//RET
		case 0x23:	rl_a(mcs51_state, op);						break;	//RL A
		case 0x24:	add_a_byte(mcs51_state, op);					break;	//ADD A, #data
		case 0x25:	add_a_mem(mcs51_state, op);					break;	//ADD A, data addr

		case 0x26:
		case 0x27:	add_a_ir(mcs51_state, op&1);					break;	//ADD A, @R0/@R1

		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:	add_a_r(mcs51_state, op&7);					break;	//ADD A, R0 to R7

		case 0x30:	jnb(mcs51_state, op);							break;	//JNB bit addr, code addr
		case 0x31:	acall(mcs51_state, op);						break;	//ACALL code addr
		case 0x32:	reti(mcs51_state, op);						break;	//RETI
		case 0x33:	rlc_a(mcs51_state, op);						break;	//RLC A
		case 0x34:	addc_a_byte(mcs51_state, op);					break;	//ADDC A, #data
		case 0x35:	addc_a_mem(mcs51_state, op);					break;	//ADDC A, data addr

		case 0x36:
		case 0x37:  addc_a_ir(mcs51_state, op&1);					break;	//ADDC A, @R0/@R1

		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:	addc_a_r(mcs51_state, op&7);					break;	//ADDC A, R0 to R7

		case 0x40:	jc(mcs51_state, op);							break;	//JC code addr
		case 0x41:	ajmp(mcs51_state, op);						break;	//AJMP code addr
		case 0x42:	RWM=1; orl_mem_a(mcs51_state, op);	RWM=0;	break;	//ORL data addr, A
		case 0x43:	RWM=1; orl_mem_byte(mcs51_state, op); RWM=0;	break;	//ORL data addr, #data
		case 0x44:	orl_a_byte(mcs51_state, op);					break;
		case 0x45:	orl_a_mem(mcs51_state, op);					break;	//ORL A, data addr

		case 0x46:
		case 0x47:	orl_a_ir(mcs51_state, op&1);					break;	//ORL A, @RO/@R1

		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
		case 0x4c:
		case 0x4d:
		case 0x4e:
		case 0x4f:	orl_a_r(mcs51_state, op&7); 					break;	//ORL A, RO to R7

		case 0x50:	jnc(mcs51_state, op);						break;	//JNC code addr
		case 0x51:  acall(mcs51_state, op);						break;	//ACALL code addr
		case 0x52:	RWM=1; anl_mem_a(mcs51_state, op); RWM=0;		break;	//ANL data addr, A
		case 0x53:	RWM=1; anl_mem_byte(mcs51_state, op); RWM=0;	break;	//ANL data addr, #data
		case 0x54:	anl_a_byte(mcs51_state, op);					break;	//ANL A, #data
		case 0x55:	anl_a_mem(mcs51_state, op);					break;	//ANL A, data addr

		case 0x56:
		case 0x57:	anl_a_ir(mcs51_state, op&1);					break;	//ANL A, @RO/@R1

		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
		case 0x5c:
		case 0x5d:
		case 0x5e:
		case 0x5f:	anl_a_r(mcs51_state, op&7);					break;	//ANL A, RO to R7

		case 0x60:	jz(mcs51_state, op);							break;	//JZ code addr
		case 0x61:	ajmp(mcs51_state, op);						break;	//AJMP code addr
		case 0x62:	RWM=1; xrl_mem_a(mcs51_state, op); RWM=0;		break;	//XRL data addr, A
		case 0x63:	RWM=1; xrl_mem_byte(mcs51_state, op); RWM=0;	break;	//XRL data addr, #data
		case 0x64:	xrl_a_byte(mcs51_state, op);					break;	//XRL A, #data
		case 0x65:	xrl_a_mem(mcs51_state, op);					break;	//XRL A, data addr

		case 0x66:
		case 0x67:	xrl_a_ir(mcs51_state, op&1);					break;	//XRL A, @R0/@R1

		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f:	xrl_a_r(mcs51_state, op&7);					break;	//XRL A, R0 to R7

		case 0x70:	jnz(mcs51_state, op);							break;	//JNZ code addr
		case 0x71:	acall(mcs51_state, op);						break;	//ACALL code addr
		case 0x72:	orl_c_bitaddr(mcs51_state, op);				break;	//ORL C, bit addr
		case 0x73:	jmp_iadptr(mcs51_state, op);					break;	//JMP @A+DPTR
		case 0x74:	mov_a_byte(mcs51_state, op);					break;	//MOV A, #data
		case 0x75:	mov_mem_byte(mcs51_state, op);				break;	//MOV data addr, #data

		case 0x76:
		case 0x77:	mov_ir_byte(mcs51_state, op&1);				break;	//MOV @R0/@R1, #data

		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f:	mov_r_byte(mcs51_state, op&7);				break;	//MOV R0 to R7, #data

		case 0x80:	sjmp(mcs51_state, op);						break;	//SJMP code addr
		case 0x81:	ajmp(mcs51_state, op);						break;	//AJMP code addr
		case 0x82:	anl_c_bitaddr(mcs51_state, op);				break;	//ANL C, bit addr
		case 0x83:	movc_a_iapc(mcs51_state, op);					break;	//MOVC A, @A + PC
		case 0x84:	div_ab(mcs51_state, op);						break;	//DIV AB
		case 0x85:	mov_mem_mem(mcs51_state, op);					break;	//MOV data addr, data addr

		case 0x86:
		case 0x87:	mov_mem_ir(mcs51_state, op&1);				break;	//MOV data addr, @R0/@R1

		case 0x88:
		case 0x89:
		case 0x8a:
		case 0x8b:
		case 0x8c:
		case 0x8d:
		case 0x8e:
		case 0x8f:	mov_mem_r(mcs51_state, op&7);					break;	//MOV data addr,R0 to R7

		case 0x90:	mov_dptr_byte(mcs51_state, op);				break;	//MOV DPTR, #data
		case 0x91:	acall(mcs51_state, op);						break;	//ACALL code addr
		case 0x92:	RWM = 1; mov_bitaddr_c(mcs51_state, op); RWM = 0; break;	//MOV bit addr, C
		case 0x93:	movc_a_iadptr(mcs51_state, op);				break;	//MOVC A, @A + DPTR
		case 0x94:	subb_a_byte(mcs51_state, op);					break;	//SUBB A, #data
		case 0x95:	subb_a_mem(mcs51_state, op);					break;	//SUBB A, data addr

		case 0x96:
		case 0x97:	subb_a_ir(mcs51_state, op&1);					break;	//SUBB A, @R0/@R1

		case 0x98:
		case 0x99:
		case 0x9a:
		case 0x9b:
		case 0x9c:
		case 0x9d:
		case 0x9e:
		case 0x9f:	subb_a_r(mcs51_state, op&7);					break;	//SUBB A, R0 to R7

		case 0xa0:	orl_c_nbitaddr(mcs51_state, op);				break;	//ORL C, /bit addr
		case 0xa1:	ajmp(mcs51_state, op);						break;	//AJMP code addr
		case 0xa2:	mov_c_bitaddr(mcs51_state, op);				break;	//MOV C, bit addr
		case 0xa3:	inc_dptr(mcs51_state, op);					break;	//INC DPTR
		case 0xa4:	mul_ab(mcs51_state, op);						break;	//MUL AB
		case 0xa5:	illegal(mcs51_state, op);						break;	//reserved

		case 0xa6:
		case 0xa7:	mov_ir_mem(mcs51_state, op&1);				break;	//MOV @R0/@R1, data addr

		case 0xa8:
		case 0xa9:
		case 0xaa:
		case 0xab:
		case 0xac:
		case 0xad:
		case 0xae:
		case 0xaf:	mov_r_mem(mcs51_state, op&7);					break;	//MOV R0 to R7, data addr

		case 0xb0:	anl_c_nbitaddr(mcs51_state, op);				break;	//ANL C,/bit addr
		case 0xb1:	acall(mcs51_state, op);						break;	//ACALL code addr
		case 0xb2:	RWM=1; cpl_bitaddr(mcs51_state, op); RWM=0; 	break;	//CPL bit addr
		case 0xb3:	cpl_c(mcs51_state, op);						break;	//CPL C
		case 0xb4:	cjne_a_byte(mcs51_state, op);					break;	//CJNE A, #data, code addr
		case 0xb5:	cjne_a_mem(mcs51_state, op);					break;	//CJNE A, data addr, code addr

		case 0xb6:
		case 0xb7:	cjne_ir_byte(mcs51_state, op&1);				break;	//CJNE @R0/@R1, #data, code addr

		case 0xb8:
		case 0xb9:
		case 0xba:
		case 0xbb:
		case 0xbc:
		case 0xbd:
		case 0xbe:
		case 0xbf:	cjne_r_byte(mcs51_state, op&7); 				break;	//CJNE R0 to R7, #data, code addr

		case 0xc0:	push(mcs51_state, op);						break;	//PUSH data addr
		case 0xc1:	ajmp(mcs51_state, op);						break;	//AJMP code addr
		case 0xc2:	RWM=1; clr_bitaddr(mcs51_state, op); RWM=0;	break;	//CLR bit addr
		case 0xc3:	clr_c(mcs51_state, op); 						break;	//CLR C
		case 0xc4:	swap_a(mcs51_state, op);						break;	//SWAP A
		case 0xc5:	xch_a_mem(mcs51_state, op);					break;	//XCH A, data addr

		case 0xc6:
		case 0xc7:  xch_a_ir(mcs51_state, op&1);					break;	//XCH A, @RO/@R1

		case 0xc8:
		case 0xc9:
		case 0xca:
		case 0xcb:
		case 0xcc:
		case 0xcd:
		case 0xce:
		case 0xcf:	xch_a_r(mcs51_state, op&7);					break;	//XCH A, RO to R7

		case 0xd0:	pop(mcs51_state, op);							break;	//POP data addr
		case 0xd1:	acall(mcs51_state, op);						break;	//ACALL code addr
		case 0xd2:	RWM=1; setb_bitaddr(mcs51_state, op); RWM=0;	break;	//SETB bit addr
		case 0xd3:	setb_c(mcs51_state, op);						break;	//SETB C
		case 0xd4:	da_a(mcs51_state, op);						break;	//DA A
		case 0xd5:	RWM=1; djnz_mem(mcs51_state, op); RWM=0;		break;	//DJNZ data addr, code addr

		case 0xd6:
		case 0xd7:  xchd_a_ir(mcs51_state, op&1);					break;	//XCHD A, @R0/@R1

		case 0xd8:
		case 0xd9:
		case 0xda:
		case 0xdb:
		case 0xdc:
		case 0xdd:
		case 0xde:
		case 0xdf:	djnz_r(mcs51_state, op&7);					break;	//DJNZ R0 to R7,code addr

		case 0xe0:	movx_a_idptr(mcs51_state, op);				break;	//MOVX A,@DPTR
		case 0xe1:	ajmp(mcs51_state, op);						break;	//AJMP code addr

		case 0xe2:
		case 0xe3:	movx_a_ir(mcs51_state, op&1);					break;	//MOVX A, @R0/@R1

		case 0xe4:	clr_a(mcs51_state, op);						break;	//CLR A
		case 0xe5:	mov_a_mem(mcs51_state, op);					break;	//MOV A, data addr
		case 0xe6:
		case 0xe7:	mov_a_ir(mcs51_state, op&1);					break;	//MOV A,@RO/@R1

		case 0xe8:
		case 0xe9:
		case 0xea:
		case 0xeb:
		case 0xec:
		case 0xed:
		case 0xee:
		case 0xef:	mov_a_r(mcs51_state, op&7);					break;	//MOV A,R0 to R7

		case 0xf0:	movx_idptr_a(mcs51_state, op);				break;	//MOVX @DPTR,A
		case 0xf1:	acall(mcs51_state, op);						break;	//ACALL code addr

		case 0xf2:
		case 0xf3:	movx_ir_a(mcs51_state, op&1);					break;	//MOVX @R0/@R1,A

		case 0xf4:	cpl_a(mcs51_state, op);						break;	//CPL A
		case 0xf5:	mov_mem_a(mcs51_state, op);					break;	//MOV data addr, A

		case 0xf6:
		case 0xf7:	mov_ir_a(mcs51_state, op&1);					break;	//MOV @R0/@R1, A

		case 0xf8:
		case 0xf9:
		case 0xfa:
		case 0xfb:
		case 0xfc:
		case 0xfd:
		case 0xfe:
		case 0xff:	mov_r_a(mcs51_state, op&7);					break;	//MOV R0 to R7, A
		default:
			illegal(mcs51_state, op);
	}
}

/***************************************************************************
    OPCODE CYCLES
***************************************************************************/

/* # of oscilations each opcode requires*/
static const UINT8 mcs51_cycles[] = {
	1,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,1,2,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,4,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,1,2,4,1,2,2,2,2,2,2,2,2,2,2,
	2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,1,1,1,2,1,1,2,2,2,2,2,2,2,2,
	2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1
};

/***********************************************************************************
 Check for pending Interrupts and process - returns # of cycles used for the int

 Note about priority & interrupting interrupts..
 1) A high priority interrupt cannot be interrupted by anything!
 2) A low priority interrupt can ONLY be interrupted by a high priority interrupt
 3) If more than 1 Interrupt Flag is set (ie, 2 simultaneous requests occur),
    the following logic works as follows:
    1) If two requests come in of different priority levels, the higher one is selected..
    2) If the requests are of the same level, an internal order is used:
        a) IEO
        b) TFO
        c) IE1
        d) TF1
        e) RI+TI
        f) TF2+EXF2
 **********************************************************************************/
static void check_irqs(mcs51_state_t *mcs51_state)
{
	UINT8 ints = (GET_IE0 | (GET_TF0<<1) | (GET_IE1<<2) | (GET_TF1<<3)
			| ((GET_RI|GET_TI)<<4));
	UINT8 int_vec = 0;
	UINT8 int_mask = 0;
	int priority_request = -1;
	int i;

	//If All Inerrupts Disabled or no pending abort..
	int_mask = (GET_EA ? IE : 0x00);

	if (mcs51_state->features & FEATURE_I8052)
		ints |= ((GET_TF2|GET_EXF2)<<5);

	if (mcs51_state->features & FEATURE_DS5002FP)
	{
		ints |= ((GET_PFW)<<5);
		mcs51_state->irq_prio[6] = 3;	/* force highest priority */
		/* mask out interrupts not enabled */
		ints &= ((int_mask & 0x1f) | ((GET_EPFW)<<5));
	}
	else
	{
		/* mask out interrupts not enabled */
		ints &= int_mask;
	}

	if (!ints)	return;

	/* CLear IDL - got enabled interrupt */
	if (mcs51_state->features & FEATURE_CMOS)
	{
		/* any interrupt terminates idle mode */
		SET_IDL(0);
		/* external interrupt wakes up */
		if (ints & (GET_IE0 | GET_IE1))
			/* but not the DS5002FP */
			if (!(mcs51_state->features & FEATURE_DS5002FP))
				SET_PD(0);
	}

	for (i=0; i<mcs51_state->num_interrupts; i++)
	{
		if (ints & (1<<i))
		{
			if (mcs51_state->irq_prio[i] > priority_request)
			{
				priority_request = mcs51_state->irq_prio[i];
				int_vec = (i<<3) | 3;
			}
		}
	}

	/* Skip the interrupt request if currently processing interrupt
     * and the new request does not have a higher priority
     */

	LOG(("Request: %d\n", priority_request));
	if (mcs51_state->irq_active && (priority_request <= mcs51_state->cur_irq_prio))
	{
		LOG(("higher or equal priority irq in progress already, skipping ...\n"));
		return;
	}

	/* also break out of jb int0,<self> loops */
	if (ROP(PC) == 0x20 && ROP_ARG(PC+1) == 0xb2 && ROP_ARG(PC+2) == 0xfd)
		PC += 3;

	//Save current pc to stack, set pc to new interrupt vector
	push_pc(mcs51_state);
	PC = int_vec;

	/* interrupts take 24 cycles */
	mcs51_state->inst_cycles += 2;

	//Set current Irq & Priority being serviced
	mcs51_state->cur_irq_prio = priority_request;
	mcs51_state->irq_active |= (1 << priority_request);

	LOG(("Take: %d %02x\n", mcs51_state->cur_irq_prio, mcs51_state->irq_active));

	//Clear any interrupt flags that should be cleared since we're servicing the irq!
	switch(int_vec) {
		case V_IE0:
			//External Int Flag only cleared when configured as Edge Triggered..
			if(GET_IT0)  /* for some reason having this, breaks alving dmd games */
				SET_IE0(0);

			/* indicate we took the external IRQ */
			if (mcs51_state->irq_callback != NULL)
				(*mcs51_state->irq_callback)(mcs51_state->device, 0);

			break;
		case V_TF0:
			//Timer 0 - Always clear Flag
			SET_TF0(0);
			break;
		case V_IE1:
			//External Int Flag only cleared when configured as Edge Triggered..
			if(GET_IT1)  /* for some reason having this, breaks alving dmd games */
				SET_IE1(0);
			/* indicate we took the external IRQ */
			if (mcs51_state->irq_callback != NULL)
				(*mcs51_state->irq_callback)(mcs51_state->device, 1);

			break;
		case V_TF1:
			//Timer 1 - Always clear Flag
			SET_TF1(0);
			break;
		case V_RITI:
			/* no flags are cleared, TI and RI remain set until reset by software */
			break;
		/* I8052 specific */
		case V_TF2:
			/* no flags are cleared according to manual */
			break;
		/* DS5002FP specific */
		/* case V_PFI:
         *  no flags are cleared, PFW is reset by software
         *  This has the same vector as V_TF2.
         */

	}
}

INLINE void burn_cycles(mcs51_state_t *mcs51_state, int cycles)
{
	/* Update Timer (if any timers are running) */
	update_timers(mcs51_state, cycles);

	/* Update Serial (only for mode 0) */
	update_serial(mcs51_state, cycles);

	/* check_irqs */
	check_irqs(mcs51_state);
}

static void mcs51_set_irq_line(mcs51_state_t *mcs51_state, int irqline, int state)
{
	/* From the manual:
     *
     * <cite>In operation all the interrupt flags are latched into the
     * interrupt control system during State 5 of every machine cycle.
     * The samples are polled during the following machine cycle.</cite>
     *
     * ==> Since we do not emulate sub-states, this assumes that the signal is present
     * for at least one cycle (12 states)
     *
     */
	UINT32 new_state = (mcs51_state->last_line_state & ~(1 << irqline)) | ((state != CLEAR_LINE) << irqline);
	/* detect 0->1 transistions */
	UINT32 tr_state = (~mcs51_state->last_line_state) & new_state;

	switch( irqline )
	{
		//External Interrupt 0
		case MCS51_INT0_LINE:
			//Line Asserted?
			if (state != CLEAR_LINE) {
				//Need cleared->active line transition? (Logical 1-0 Pulse on the line) - CLEAR->ASSERT Transition since INT0 active lo!
				if (GET_IT0) {
					if (GET_BIT(tr_state, MCS51_INT0_LINE))
						SET_IE0(1);
				}
				else
					SET_IE0(1);		//Nope, just set it..
			}
			else
			{
				if (!GET_IT0) /* clear if level triggered */
					SET_IE0(0);
			}

			break;

		//External Interrupt 1
		case MCS51_INT1_LINE:

			//Line Asserted?
			if (state != CLEAR_LINE) {
				//Need cleared->active line transition? (Logical 1-0 Pulse on the line) - CLEAR->ASSERT Transition since INT1 active lo!
				if(GET_IT1){
					if (GET_BIT(tr_state, MCS51_INT1_LINE))
						SET_IE1(1);
				}
				else
					SET_IE1(1);		//Nope, just set it..
			}
			else
			{
				if (!GET_IT1) /* clear if level triggered */
					SET_IE1(0);
			}
			break;

		case MCS51_T0_LINE:
			if (GET_BIT(tr_state, MCS51_T0_LINE) && GET_TR0)
				mcs51_state->t0_cnt++;
			break;

		case MCS51_T1_LINE:
			if (GET_BIT(tr_state, MCS51_T1_LINE) && GET_TR1)
				mcs51_state->t1_cnt++;
			break;

		case MCS51_T2_LINE:
			if (mcs51_state->features & FEATURE_I8052)
			{
				if (GET_BIT(tr_state, MCS51_T2_LINE) && GET_TR1)
					mcs51_state->t2_cnt++;
			}
			else
				fatalerror("mcs51: Trying to set T2_LINE on a non I8052 type cpu.\n");
			break;

		case MCS51_T2EX_LINE:
			if (mcs51_state->features & FEATURE_I8052)
			{
				if (GET_BIT(tr_state, MCS51_T2EX_LINE))
				{
					SET_EXF2(1);
					mcs51_state->t2ex_cnt++;
				}
			}
			else
				fatalerror("mcs51: Trying to set T2EX_LINE on a non I8052 type cpu.\n");
			break;

		case MCS51_RX_LINE: /* Serial Port Receive */
			/* Is the enable flags for this interrupt set? */
			if (state != CLEAR_LINE)
			{
				serial_receive(mcs51_state);
			}
			break;

		/* Power Fail Interrupt */
		case DS5002FP_PFI_LINE:
			if (mcs51_state->features & FEATURE_DS5002FP)
			{
				/* Need cleared->active line transition? (Logical 1-0 Pulse on the line) - CLEAR->ASSERT Transition since INT1 active lo! */
				if (GET_BIT(tr_state, MCS51_INT1_LINE))
					SET_PFW(1);
			}
			else
				fatalerror("mcs51: Trying to set DS5002FP_PFI_LINE on a non DS5002FP type cpu.\n");
			break;
	}
	mcs51_state->last_line_state = new_state;
}

/* Execute cycles - returns number of cycles actually run */
static CPU_EXECUTE( mcs51 )
{
	mcs51_state_t *mcs51_state = get_safe_token(device);
	UINT8 op;

	update_ptrs(mcs51_state);

	/* external interrupts may have been set since we last checked */
	mcs51_state->inst_cycles = 0;
	check_irqs(mcs51_state);

	/* if in powerdown, just return */
	if ((mcs51_state->features & FEATURE_CMOS) && GET_PD)
	{
		mcs51_state->icount = 0;
		return;
	}

	mcs51_state->icount -= mcs51_state->inst_cycles;
	burn_cycles(mcs51_state, mcs51_state->inst_cycles);

	if ((mcs51_state->features & FEATURE_CMOS) && GET_IDL)
	{
		do
		{
			/* burn the cycles */
			mcs51_state->icount--;
			burn_cycles(mcs51_state, 1);
		} while( mcs51_state->icount > 0 );
		return;
	}

	do
	{
		/* Read next opcode */
		PPC = PC;
		debugger_instruction_hook(device, PC);
		op = mcs51_state->direct->read_decrypted_byte(PC++);

		/* process opcode and count cycles */
		mcs51_state->inst_cycles = mcs51_cycles[op];
		execute_op(mcs51_state, op);

		/* burn the cycles */
		mcs51_state->icount -= mcs51_state->inst_cycles;

		/* if in powerdown, just return */
		if ((mcs51_state->features & FEATURE_CMOS) && GET_PD)
			return;

		burn_cycles(mcs51_state, mcs51_state->inst_cycles);

		/* decrement the timed access window */
		if (mcs51_state->features & FEATURE_DS5002FP)
			mcs51_state->ds5002fp.ta_window = (mcs51_state->ds5002fp.ta_window ? (mcs51_state->ds5002fp.ta_window - 1) : 0x00);

		/* If the chip entered in idle mode, end the loop */
		if ((mcs51_state->features & FEATURE_CMOS) && GET_IDL)
			return;

	} while( mcs51_state->icount > 0 );
}


/****************************************************************************
 * MCS51/8051 Section
 ****************************************************************************/

static void mcs51_sfr_write(mcs51_state_t *mcs51_state, size_t offset, UINT8 data)
{
	/* update register */
	assert(offset >= 0x80 && offset <= 0xff);

	switch (offset)
	{
		case ADDR_P0:	OUT(MCS51_PORT_P0,data);			break;
		case ADDR_P1:	OUT(MCS51_PORT_P1,data);			break;
		case ADDR_P2:	OUT(MCS51_PORT_P2,data);			break;
		case ADDR_P3:	OUT(MCS51_PORT_P3,data);			break;
		case ADDR_SBUF:	serial_transmit(mcs51_state, data); 		break;
		case ADDR_PSW:	SET_PARITY();						break;
		case ADDR_ACC:	SET_PARITY();						break;
		case ADDR_IP:	update_irq_prio(mcs51_state, data, 0);	break;
		/* R_SBUF = data;        //This register is used only for "Receiving data coming in!" */

		case ADDR_B:
		case ADDR_SP:
		case ADDR_DPL:
		case ADDR_DPH:
		case ADDR_PCON:
		case ADDR_TCON:
		case ADDR_TMOD:
		case ADDR_IE:
		case ADDR_TL0:
		case ADDR_TL1:
		case ADDR_TH0:
		case ADDR_TH1:
		case ADDR_SCON:
			break;
		default:
			LOG(("mcs51 '%s': attemping to write to an invalid/non-implemented SFR address: %x at 0x%04x, data=%x\n", mcs51_state->device->tag(), (UINT32)offset,PC,data));
			/* no write in this case according to manual */
			return;
	}
	mcs51_state->data->write_byte((size_t)offset | 0x100, data);
}

static UINT8 mcs51_sfr_read(mcs51_state_t *mcs51_state, size_t offset)
{
	assert(offset >= 0x80 && offset <= 0xff);

	switch (offset)
	{
		/* Read/Write/Modify operations read the port latch ! */
		/* Move to memory map */
		case ADDR_P0:	return RWM ? P0 : P0 & IN(MCS51_PORT_P0);
		case ADDR_P1:	return RWM ? P1 : P1 & IN(MCS51_PORT_P1);
		case ADDR_P2:	return RWM ? P2 : P2 & IN(MCS51_PORT_P2);
		case ADDR_P3:	return RWM ? P3 : P3 & IN(MCS51_PORT_P3);

		case ADDR_PSW:
		case ADDR_ACC:
		case ADDR_B:
		case ADDR_SP:
		case ADDR_DPL:
		case ADDR_DPH:
		case ADDR_PCON:
		case ADDR_TCON:
		case ADDR_TMOD:
		case ADDR_TL0:
		case ADDR_TL1:
		case ADDR_TH0:
		case ADDR_TH1:
		case ADDR_SCON:
		case ADDR_SBUF:
		case ADDR_IE:
		case ADDR_IP:
			return mcs51_state->data->read_byte((size_t) offset | 0x100);
		/* Illegal or non-implemented sfr */
		default:
			LOG(("mcs51 '%s': attemping to read an invalid/non-implemented SFR address: %x at 0x%04x\n", mcs51_state->device->tag(), (UINT32)offset,PC));
			/* according to the manual, the read may return random bits */
			return 0xff;
	}
}


static CPU_INIT( mcs51 )
{
	mcs51_state_t *mcs51_state = get_safe_token(device);

	mcs51_state->irq_callback = irqcallback;
	mcs51_state->device = device;

	mcs51_state->program = device->space(AS_PROGRAM);
	mcs51_state->direct = &mcs51_state->program->direct();
	mcs51_state->data = device->space(AS_DATA);
	mcs51_state->io = device->space(AS_IO);

	mcs51_state->features = FEATURE_NONE;
	mcs51_state->ram_mask = 0x7F;			/* 128 bytes of ram */
	mcs51_state->num_interrupts = 5;			/* 5 interrupts */
	mcs51_state->sfr_read = mcs51_sfr_read;
	mcs51_state->sfr_write = mcs51_sfr_write;

	/* ensure these pointers are set before get_info is called */
	update_ptrs(mcs51_state);

	/* Save states */

	device->save_item(NAME(mcs51_state->ppc));
	device->save_item(NAME(mcs51_state->pc));
	device->save_item(NAME(mcs51_state->rwm) );
	device->save_item(NAME(mcs51_state->cur_irq_prio) );
	device->save_item(NAME(mcs51_state->last_line_state) );
	device->save_item(NAME(mcs51_state->t0_cnt) );
	device->save_item(NAME(mcs51_state->t1_cnt) );
	device->save_item(NAME(mcs51_state->t2_cnt) );
	device->save_item(NAME(mcs51_state->t2ex_cnt) );
	device->save_item(NAME(mcs51_state->recalc_parity) );
	device->save_item(NAME(mcs51_state->irq_prio) );
	device->save_item(NAME(mcs51_state->irq_active) );
}

static CPU_INIT( i80c51 )
{
	mcs51_state_t *mcs51_state = get_safe_token(device);
	CPU_INIT_CALL(mcs51);
	mcs51_state->features |= FEATURE_CMOS;
}

/* Reset registers to the initial values */
static CPU_RESET( mcs51 )
{
	mcs51_state_t *mcs51_state = get_safe_token(device);

	update_ptrs(mcs51_state);

	mcs51_state->last_line_state = 0;
	mcs51_state->t0_cnt = 0;
	mcs51_state->t1_cnt = 0;
	mcs51_state->t2_cnt = 0;
	mcs51_state->t2ex_cnt = 0;
	/* Flag as NO IRQ in Progress */
	mcs51_state->irq_active = 0;
	mcs51_state->cur_irq_prio = -1;

	/* these are all defined reset states */
	PC = 0;
	SP = 0x7;
	SET_PSW(0);
	SET_ACC(0);
	DPH = 0;
	DPL = 0;
	B = 0;
	IP = 0;
	update_irq_prio(mcs51_state, IP, 0);
	IE = 0;
	SCON = 0;
	TCON = 0;
	TMOD = 0;
	PCON = 0;
	TH1 = 0;
	TH0 = 0;
	TL1 = 0;
	TL0 = 0;
	/* set the port configurations to all 1's */
	SET_P3(0xff);
	SET_P2(0xff);
	SET_P1(0xff);
	SET_P0(0xff);

	/* 8052 Only registers */
	if (mcs51_state->features & FEATURE_I8052)
	{
		T2CON = 0;
		RCAP2L = 0;
		RCAP2H = 0;
		TL2 = 0;
		TH2 = 0;
	}

	/* 80C52 Only registers */
	if (mcs51_state->features & FEATURE_I80C52)
	{
		IPH = 0;
		update_irq_prio(mcs51_state, IP, IPH);
		SADDR = 0;
		SADEN = 0;
	}

	/* DS5002FP Only registers */
	if (mcs51_state->features & FEATURE_DS5002FP)
	{
		// set initial values (some of them are set using the bootstrap loader)
		PCON = 0;
		MCON = mcs51_state->ds5002fp.config->mcon & 0xfb;
		RPCTL = mcs51_state->ds5002fp.config->rpctl & 0x01;
		RPS = 0;
		RNR = 0;
		CRCR = mcs51_state->ds5002fp.config->crc & 0xf0;
		CRCL = 0;
		CRCH = 0;
		TA = 0;

		// set internal CPU state
		mcs51_state->ds5002fp.previous_ta = 0;
		mcs51_state->ds5002fp.ta_window = 0;
		mcs51_state->ds5002fp.range = (GET_RG1 << 1) | GET_RG0;
	}

}

/* Shut down CPU core */
static CPU_EXIT( mcs51 )
{
	/* nothing to do */
}

/****************************************************************************
 * 8052 Section
 ****************************************************************************/

static void i8052_sfr_write(mcs51_state_t *mcs51_state, size_t offset, UINT8 data)
{
	switch (offset)
	{
		/* 8052 family specific */
		case ADDR_T2CON:
		case ADDR_RCAP2L:
		case ADDR_RCAP2H:
		case ADDR_TL2:
		case ADDR_TH2:
			mcs51_state->data->write_byte((size_t) offset | 0x100, data);
			break;

		default:
			mcs51_sfr_write(mcs51_state, offset, data);
	}
}

static UINT8 i8052_sfr_read(mcs51_state_t *mcs51_state, size_t offset)
{
	switch (offset)
	{
		/* 8052 family specific */
		case ADDR_T2CON:
		case ADDR_RCAP2L:
		case ADDR_RCAP2H:
		case ADDR_TL2:
		case ADDR_TH2:
			return mcs51_state->data->read_byte((size_t) offset | 0x100);
		default:
			return mcs51_sfr_read(mcs51_state, offset);
	}
}

static CPU_INIT( i8052 )
{
	mcs51_state_t *mcs51_state = get_safe_token(device);
	CPU_INIT_CALL(mcs51);

	mcs51_state->ram_mask = 0xFF;			/* 256 bytes of ram */
	mcs51_state->num_interrupts = 6;			/* 6 interrupts */

	mcs51_state->features |= FEATURE_I8052;
	mcs51_state->sfr_read = i8052_sfr_read;
	mcs51_state->sfr_write = i8052_sfr_write;
}

/****************************************************************************
 * 80C52 Section
 ****************************************************************************/

static void i80c52_sfr_write(mcs51_state_t *mcs51_state, size_t offset, UINT8 data)
{
	switch (offset)
	{
		/* 80c52 family specific */
		case ADDR_IP:
			update_irq_prio(mcs51_state, data, IPH);
			break;
		case ADDR_IPH:
			update_irq_prio(mcs51_state, IP, data);
			break;
		case ADDR_SADDR:
		case ADDR_SADEN:
			break;

		default:
			i8052_sfr_write(mcs51_state, offset, data);
			return;
	}
	mcs51_state->data->write_byte((size_t) offset | 0x100, data);
}

static UINT8 i80c52_sfr_read(mcs51_state_t *mcs51_state, size_t offset)
{
	switch (offset)
	{
		/* 80c52 family specific */
		case ADDR_IPH:
		case ADDR_SADDR:
		case ADDR_SADEN:
			return mcs51_state->data->read_byte((size_t) offset | 0x100);
		default:
			return i8052_sfr_read(mcs51_state, offset);
	}
}

static CPU_INIT( i80c52 )
{
	mcs51_state_t *mcs51_state = get_safe_token(device);
	CPU_INIT_CALL(i8052);

	mcs51_state->features |= (FEATURE_I80C52 | FEATURE_CMOS);
	mcs51_state->sfr_read = i80c52_sfr_read;
	mcs51_state->sfr_write = i80c52_sfr_write;
}

static CPU_INIT( i80c31 )
{
	mcs51_state_t *mcs51_state = get_safe_token(device);
	CPU_INIT_CALL(i8052);

	mcs51_state->ram_mask = 0x7F;			/* 128 bytes of ram */
}

/****************************************************************************
 * DS5002FP Section
 ****************************************************************************/


#define DS5_LOGW(a, d)	LOG(("ds5002fp '%s': write to  " # a " register at 0x%04x, data=%x\n", mcs51_state->device->tag(), PC, d))
#define DS5_LOGR(a, d)	LOG(("ds5002fp '%s': read from " # a " register at 0x%04x\n", mcs51_state->device->tag(), PC))

INLINE UINT8 ds5002fp_protected(mcs51_state_t *mcs51_state, size_t offset, UINT8 data, UINT8 ta_mask, UINT8 mask)
{
	UINT8 is_timed_access;

	is_timed_access = (mcs51_state->ds5002fp.ta_window > 0) && (TA == 0x55);
	if (is_timed_access)
	{
		ta_mask = 0xff;
	}
	data = (mcs51_state->sfr_ram[offset] & (~ta_mask)) | (data & ta_mask);
	return (mcs51_state->sfr_ram[offset] & (~mask)) | (data & mask);
}

static void ds5002fp_sfr_write(mcs51_state_t *mcs51_state, size_t offset, UINT8 data)
{
	switch (offset)
	{

		case ADDR_TA:
			mcs51_state->ds5002fp.previous_ta = TA;
			/*  init the time window after having wrote 0xaa */
			if ((data == 0xaa) && (mcs51_state->ds5002fp.ta_window == 0))
			{
				mcs51_state->ds5002fp.ta_window = 6; /* 4*12 + 2*12 */
				LOG(("ds5002fp '%s': TA window initiated at 0x%04x\n", mcs51_state->device->tag(), PC));
			}
			break;
		case ADDR_MCON: 	data = ds5002fp_protected(mcs51_state, ADDR_MCON, data, 0x0f, 0xf7);	DS5_LOGW(MCON, data); break;
		case ADDR_RPCTL:	data = ds5002fp_protected(mcs51_state, ADDR_RPCTL, data, 0xef, 0xfe); DS5_LOGW(RPCTL, data); break;
		case ADDR_CRCR:		data = ds5002fp_protected(mcs51_state, ADDR_CRCR, data, 0xff, 0x0f);	DS5_LOGW(CRCR, data);	break;
		case ADDR_PCON:		data = ds5002fp_protected(mcs51_state, ADDR_PCON, data, 0xb9, 0xff); break;
		case ADDR_IP:		data = ds5002fp_protected(mcs51_state, ADDR_IP, data, 0x7f, 0xff);	break;
		case ADDR_CRCL:		DS5_LOGW(CRCL, data);									break;
		case ADDR_CRCH:		DS5_LOGW(CRCH, data);									break;
		case ADDR_RNR:		DS5_LOGW(RNR, data);									break;
		case ADDR_RPS:		DS5_LOGW(RPS, data);									break;
		default:
			mcs51_sfr_write(mcs51_state, offset, data);
			return;
	}
	mcs51_state->data->write_byte((size_t) offset | 0x100, data);
}

static UINT8 ds5002fp_sfr_read(mcs51_state_t *mcs51_state, size_t offset)
{
	switch (offset)
	{
		case ADDR_CRCR: 	DS5_LOGR(CRCR, data);		break;
		case ADDR_CRCL: 	DS5_LOGR(CRCL, data);		break;
		case ADDR_CRCH: 	DS5_LOGR(CRCH, data);		break;
		case ADDR_MCON: 	DS5_LOGR(MCON, data);		break;
		case ADDR_TA:		DS5_LOGR(TA, data);			break;
		case ADDR_RNR:		DS5_LOGR(RNR, data);		break;
		case ADDR_RPCTL:	DS5_LOGR(RPCTL, data);		break;
		case ADDR_RPS:		DS5_LOGR(RPS, data);		break;
		case ADDR_PCON:
			SET_PFW(0);		/* reset PFW flag */
			return mcs51_sfr_read(mcs51_state, offset);
		default:
			return mcs51_sfr_read(mcs51_state, offset);
	}
	return mcs51_state->data->read_byte((size_t) offset | 0x100);
}

static CPU_INIT( ds5002fp )
{
	/* default configuration */
	static const ds5002fp_config default_config = { 0x00, 0x00, 0x00 };
	const ds5002fp_config *sconfig = device->static_config() ? (const ds5002fp_config *)device->static_config() : &default_config;
	mcs51_state_t *mcs51_state = get_safe_token(device);

	CPU_INIT_CALL( mcs51 );

	mcs51_state->ds5002fp.config = sconfig;
	mcs51_state->features |= (FEATURE_DS5002FP | FEATURE_CMOS);
	mcs51_state->sfr_read = ds5002fp_sfr_read;
	mcs51_state->sfr_write = ds5002fp_sfr_write;

	device->save_item(NAME(mcs51_state->ds5002fp.previous_ta) );
	device->save_item(NAME(mcs51_state->ds5002fp.ta_window) );
	device->save_item(NAME(mcs51_state->ds5002fp.range) );

}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START(program_12bit, AS_PROGRAM, 8, legacy_cpu_device)
	AM_RANGE(0x00, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(program_13bit, AS_PROGRAM, 8, legacy_cpu_device)
	AM_RANGE(0x00, 0x1fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_7bit, AS_DATA, 8, legacy_cpu_device)
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x0100, 0x01ff) AM_RAM /* SFR */
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_8bit, AS_DATA, 8, legacy_cpu_device)
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x01ff) AM_RAM /* SFR */
ADDRESS_MAP_END


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( mcs51 )
{
	mcs51_state_t *mcs51_state = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:							PC = info->i;							break;
		case CPUINFO_INT_SP:							SP = info->i;		    				break;

		case CPUINFO_INT_INPUT_STATE + MCS51_INT0_LINE:	mcs51_set_irq_line(mcs51_state, MCS51_INT0_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + MCS51_INT1_LINE:	mcs51_set_irq_line(mcs51_state, MCS51_INT1_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + MCS51_T0_LINE:	mcs51_set_irq_line(mcs51_state, MCS51_T0_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + MCS51_T1_LINE:	mcs51_set_irq_line(mcs51_state, MCS51_T1_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + MCS51_RX_LINE:	mcs51_set_irq_line(mcs51_state, MCS51_RX_LINE, info->i); break;

		case CPUINFO_INT_REGISTER + MCS51_PC:			PC = info->i;								break;
		case CPUINFO_INT_REGISTER + MCS51_SP:			SP = info->i;							break;
		case CPUINFO_INT_REGISTER + MCS51_PSW:			SET_PSW(info->i);							break;
		case CPUINFO_INT_REGISTER + MCS51_ACC:			SET_ACC(info->i);							break;
		case CPUINFO_INT_REGISTER + MCS51_B:			B = info->i;								break;
		case CPUINFO_INT_REGISTER + MCS51_DPH:			DPH = info->i;							break;
		case CPUINFO_INT_REGISTER + MCS51_DPL:			DPL = info->i;							break;
		case CPUINFO_INT_REGISTER + MCS51_IE:			IE = info->i;							break;
		case CPUINFO_INT_REGISTER + MCS51_R0:			SET_REG(0, info->i); break;
		case CPUINFO_INT_REGISTER + MCS51_R1:			SET_REG(1, info->i); break;
		case CPUINFO_INT_REGISTER + MCS51_R2:			SET_REG(2, info->i); break;
		case CPUINFO_INT_REGISTER + MCS51_R3:			SET_REG(3, info->i); break;
		case CPUINFO_INT_REGISTER + MCS51_R4:			SET_REG(4, info->i); break;
		case CPUINFO_INT_REGISTER + MCS51_R5:			SET_REG(5, info->i); break;
		case CPUINFO_INT_REGISTER + MCS51_R6:			SET_REG(6, info->i); break;
		case CPUINFO_INT_REGISTER + MCS51_R7:			SET_REG(7, info->i); break;
		case CPUINFO_INT_REGISTER + MCS51_RB:			SET_RS(info->i); break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static CPU_GET_INFO( mcs51 )
{
	mcs51_state_t *mcs51_state = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(mcs51_state_t);				break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;								break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;						break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;								break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 12;								break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;								break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 5;								break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;								break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 20; /* rough guess */				break;
		case CPUINFO_INT_INPUT_LINES:       			info->i = 3;								break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;						break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16;						break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;						break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 8;						break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 9; /* due to sfr mapping */					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;						break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:	info->i = 8;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:	info->i = 18; /* 128k for ds5002fp */							break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:	info->i = 0;							break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = PPC;								break;
		case CPUINFO_INT_PC:							info->i = PC;								break;
		case CPUINFO_INT_SP:							info->i = SP;								break;

		case CPUINFO_INT_REGISTER + MCS51_PC:			info->i = PC;								break;
		case CPUINFO_INT_REGISTER + MCS51_SP:			info->i = SP;								break;
		case CPUINFO_INT_REGISTER + MCS51_PSW:			info->i = PSW;								break;
		case CPUINFO_INT_REGISTER + MCS51_ACC:			info->i = ACC;								break;
		case CPUINFO_INT_REGISTER + MCS51_B:			info->i = B;								break;
		case CPUINFO_INT_REGISTER + MCS51_DPH:			info->i = DPH;								break;
		case CPUINFO_INT_REGISTER + MCS51_DPL:			info->i = DPL;								break;
		case CPUINFO_INT_REGISTER + MCS51_IE:			info->i = IE;								break;
		case CPUINFO_INT_REGISTER + MCS51_R0:			info->i = R_REG(0);							break;
		case CPUINFO_INT_REGISTER + MCS51_R1:			info->i = R_REG(1); 						break;
		case CPUINFO_INT_REGISTER + MCS51_R2:			info->i = R_REG(2); 						break;
		case CPUINFO_INT_REGISTER + MCS51_R3:			info->i = R_REG(3); 						break;
		case CPUINFO_INT_REGISTER + MCS51_R4:			info->i = R_REG(4); 						break;
		case CPUINFO_INT_REGISTER + MCS51_R5:			info->i = R_REG(5); 						break;
		case CPUINFO_INT_REGISTER + MCS51_R6:			info->i = R_REG(6); 						break;
		case CPUINFO_INT_REGISTER + MCS51_R7:			info->i = R_REG(7); 						break;
		case CPUINFO_INT_REGISTER + MCS51_RB:			info->i = R_REG(8); 						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(mcs51);				break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(mcs51);			break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(mcs51);		break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(mcs51);			break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(mcs51);	break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;							break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i8051);				break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &mcs51_state->icount;				break;

		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map8 = NULL;	break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:    info->internal_map8 = NULL;	break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_IO:      info->internal_map8 = NULL;	break;

		case DEVINFO_STR_NAME:							strcpy(info->s, "I8051");					break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "MCS-51");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Steve Ellenoff"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				PSW & 0x80 ? 'C':'.',
				PSW & 0x40 ? 'A':'.',
				PSW & 0x20 ? 'F':'.',
				PSW & 0x10 ? '0':'.',
				PSW & 0x08 ? '1':'.',
				PSW & 0x04 ? 'V':'.',
				PSW & 0x02 ? '?':'.',
				PSW & 0x01 ? 'P':'.');
			break;

		case CPUINFO_STR_REGISTER + MCS51_PC:       	sprintf(info->s, "PC:%04X", mcs51_state->pc);		break;
		case CPUINFO_STR_REGISTER + MCS51_SP:       	sprintf(info->s, "SP:%02X", SP);			break;
		case CPUINFO_STR_REGISTER + MCS51_PSW:      	sprintf(info->s, "PSW:%02X", PSW);			break;
		case CPUINFO_STR_REGISTER + MCS51_ACC:      	sprintf(info->s, "A:%02X", ACC);			break;
		case CPUINFO_STR_REGISTER + MCS51_B:        	sprintf(info->s, "B:%02X", B);				break;
		case CPUINFO_STR_REGISTER + MCS51_DPH:      	sprintf(info->s, "DPH:%02X", DPH);			break;
		case CPUINFO_STR_REGISTER + MCS51_DPL:      	sprintf(info->s, "DPL:%02X", DPL);			break;
		case CPUINFO_STR_REGISTER + MCS51_IE:       	sprintf(info->s, "IE:%02X", IE);			break;
		case CPUINFO_STR_REGISTER + MCS51_R0:       	sprintf(info->s, "R0:%02X", R_REG(0));		break;
		case CPUINFO_STR_REGISTER + MCS51_R1:       	sprintf(info->s, "R1:%02X", R_REG(1));		break;
		case CPUINFO_STR_REGISTER + MCS51_R2:       	sprintf(info->s, "R2:%02X", R_REG(2));		break;
		case CPUINFO_STR_REGISTER + MCS51_R3:			sprintf(info->s, "R3:%02X", R_REG(3));		break;
		case CPUINFO_STR_REGISTER + MCS51_R4:			sprintf(info->s, "R4:%02X", R_REG(4));		break;
		case CPUINFO_STR_REGISTER + MCS51_R5:			sprintf(info->s, "R5:%02X", R_REG(5));		break;
		case CPUINFO_STR_REGISTER + MCS51_R6:			sprintf(info->s, "R6:%02X", R_REG(6));		break;
		case CPUINFO_STR_REGISTER + MCS51_R7:			sprintf(info->s, "R7:%02X", R_REG(7));		break;
		case CPUINFO_STR_REGISTER + MCS51_RB:			sprintf(info->s, "RB:%02X", ((PSW & 0x18)>>3)); break;
	}
}

/**************************************************************************
 * Specific get_info
 **************************************************************************/

CPU_GET_INFO( i8031 )
{
	switch (state)
	{
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:    info->internal_map8 = ADDRESS_MAP_NAME(data_7bit);	break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "I8031");					break;
		default:										CPU_GET_INFO_CALL(mcs51);					break;
	}
	/* --- the following bits of info are returned as NULL-terminated strings --- */
}

CPU_GET_INFO( i8051 )
{
	switch (state)
	{
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map8 = ADDRESS_MAP_NAME(program_12bit);	break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:    info->internal_map8 = ADDRESS_MAP_NAME(data_7bit);	break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "I8051");					break;
		default:										CPU_GET_INFO_CALL(mcs51);					break;
	}
	/* --- the following bits of info are returned as NULL-terminated strings --- */
}

CPU_GET_INFO( i8032 )
{
	switch (state)
	{
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i8052);			break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:    info->internal_map8 = ADDRESS_MAP_NAME(data_8bit);	break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i8052);				break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "I8032");					break;
		default:										CPU_GET_INFO_CALL(mcs51);					break;
	}
}

CPU_GET_INFO( i8052 )
{
	switch (state)
	{
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i8052);			break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map8 = ADDRESS_MAP_NAME(program_13bit);	break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:    info->internal_map8 = ADDRESS_MAP_NAME(data_8bit);	break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i8052);				break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "I8052");					break;
		default:										CPU_GET_INFO_CALL(mcs51);					break;
	}
}

CPU_GET_INFO( i8751 )
{
	switch (state)
	{
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map8 = ADDRESS_MAP_NAME(program_12bit);	break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:    info->internal_map8 = ADDRESS_MAP_NAME(data_7bit);	break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "I8751");					break;
		default:										CPU_GET_INFO_CALL(mcs51);					break;
	}
}

CPU_GET_INFO( i8752 )
{
	switch (state)
	{
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i8052);			break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map8 = ADDRESS_MAP_NAME(program_13bit);	break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:    info->internal_map8 = ADDRESS_MAP_NAME(data_8bit);	break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i8052);				break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "I8752");					break;
		default:										CPU_GET_INFO_CALL(mcs51);					break;
	}
}

/**************************************************************************
 * CMOS get_info
 **************************************************************************/

CPU_GET_INFO( i80c31 )
{
	/* according to PHILIPS datasheet this is a stripped down version
     * of i80c52 with 128 bytes internal ram */
	switch (state)
	{
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i80c31);			break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map8 = NULL;	break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:    info->internal_map8 = ADDRESS_MAP_NAME(data_7bit);	break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i80c51);			break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "I80C31");					break;
		default:										CPU_GET_INFO_CALL(i8031);					break;
	}
}

CPU_GET_INFO( i80c51 )
{
	switch (state)
	{
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i80c51);			break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i80c51);			break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "I80C51");					break;
		default:										CPU_GET_INFO_CALL(i8051);					break;
	}
}

CPU_GET_INFO( i80c32 )
{
	switch (state)
	{
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i80c52);			break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i80c52);			break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "I80C32");					break;
		default:										CPU_GET_INFO_CALL(i8032);					break;
	}
}

CPU_GET_INFO( i80c52 )
{
	switch (state)
	{
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i80c52);			break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "I80C52");					break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i80c52);			break;
		default:										CPU_GET_INFO_CALL(i8052);					break;
	}
}

CPU_GET_INFO( i87c51 )
{
	switch (state)
	{
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i80c51);			break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "I87C51");					break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i80c51);			break;
		default:										CPU_GET_INFO_CALL(i8751);					break;
	}
}

CPU_GET_INFO( i87c52 )
{
	switch (state)
	{
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i80c52);			break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "I87C52");					break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i80c52);			break;
		default:										CPU_GET_INFO_CALL(i8752);					break;
	}
}

/**************************************************************************
 * Other variants get_info
 **************************************************************************/

CPU_GET_INFO( at89c4051 )
{
	switch (state)
	{
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i80c51);			break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "AT89C4051");				break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i80c51);			break;
		default:										CPU_GET_INFO_CALL(i8051);					break;
	}
}

CPU_GET_INFO( ds5002fp )
{
	switch (state)
	{
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(ds5002fp);		break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "DS5002FP");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Dallas");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Manuel Abadia"); break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(ds5002fp);			break;
		default:										CPU_GET_INFO_CALL(i8051);					break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(I8031, i8031);
DEFINE_LEGACY_CPU_DEVICE(I8032, i8032);
DEFINE_LEGACY_CPU_DEVICE(I8051, i8051);
DEFINE_LEGACY_CPU_DEVICE(I8751, i8751);
DEFINE_LEGACY_CPU_DEVICE(I8052, i8052);
DEFINE_LEGACY_CPU_DEVICE(I8752, i8752);
DEFINE_LEGACY_CPU_DEVICE(I80C31, i80c31);
DEFINE_LEGACY_CPU_DEVICE(I80C51, i80c51);
DEFINE_LEGACY_CPU_DEVICE(I87C51, i87c51);
DEFINE_LEGACY_CPU_DEVICE(I80C32, i80c32);
DEFINE_LEGACY_CPU_DEVICE(I80C52, i80c52);
DEFINE_LEGACY_CPU_DEVICE(I87C52, i87c52);
DEFINE_LEGACY_CPU_DEVICE(AT89C4051, at89c4051);
DEFINE_LEGACY_CPU_DEVICE(DS5002FP, ds5002fp);
