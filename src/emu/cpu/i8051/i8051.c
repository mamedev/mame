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

/******************************************************************************
 *  Notes:
 *        *Important*: Internal ROM needs to be treated the same as external rom by the programmer
 *                     creating the driver (ie, use standard cpu rom region)
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
 *        NOW Implemented: RAM paging using hardware configured addressing...
 *        (July 28,2004)   the "MOVX a,@R0/R1" and "MOVX @R0/R1,a" commands can use any of the other ports
 *                 to output a page offset into external ram, but it is totally based on the hardware setup.
 *
 *        Timing needs to be implemented via MAME timers perhaps?
 *
 *        October, 2008, Couriersud - Major rewrite
 *
 *****************************************************************************/

/* TODO: Varios
 *  - EA pin - defined by architecture, must implement:
 *    1 means external access, bypassing internal ROM
 *  - T0 output clock ?
 *
 * - Full Timer support (all modes)
 * - Implement cmos features
 * - Fix serial communication - This is a big hack (but working) right now.
 * - Implement 87C751 in sslam.c
 * - Fix cardline.c
 * - Fix sslam.c and cardline.c
 *      most likely due to different behaviour of I/O pins. The boards
 *      actually use 80CXX, i.e. CMOS versions.
 *      "Normal" 805X will return a 0 if reading from a output port which has
 *      a 0 written to it's latch. At least cardline expects a 1 here.
 *
 * Done:
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

#include "debugger.h"
#include "deprecat.h"
#include "i8051.h"

#define VERBOSE 1

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	FEATURE_NONE			= 0,
	FEATURE_I8052_UART		= 1,
	FEATURE_CMOS_IDLE		= 2,
	FEATURE_CMOS_POWERDOWN	= 4,
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

typedef struct _mcs51_regs mcs51_regs;
struct _mcs51_regs
{
	//Internal stuff
	UINT16	ppc;			//previous pc
	UINT16	pc;				//current pc
	UINT16	features;		//features of this cpu
	UINT8	cur_irq;		//Holds value of any current IRQ being serviced
	UINT8	irq_priority;	//Holds value of the current IRQ Priority Level
	UINT8	rwm;			//Signals that the current instruction is a read/write/modify instruction

	int		inst_cycles;		/* cycles for the current instruction */
	int		ram_mask;  			/* second ram bank for indirect access available ? */
	int		num_interrupts;		/* number of interrupts supported */
	int		recalc_parity;		/* recalculate parity before next instruction */
	int		last_line_state;	/* last state of input lines line */
	int		t0_cnt;				/* number of 0->1 transistions on T0 line */
	int		t1_cnt;				/* number of 0->1 transistions on T1 line */
	int		t2_cnt;				/* number of 0->1 transistions on T2 line */
	int		t2ex_cnt;			/* number of 0->1 transistions on T2EX line */

	mcs51_uart uart;			/* internal uart */

	/* Internal Ram */
	UINT8	*internal_ram;		/* 128 RAM (8031/51) + 128 RAM in second bank (8032/52) */
	UINT8	*sfr_ram;		/* 128 SFR - these are in 0x80 - 0xFF */

	// SFR Callbacks
	void 	(*sfr_write)(size_t offset, UINT8 data);
	UINT8	(*sfr_read)(size_t offset);

	//Interrupt Callback
	int 	(*irq_callback)(int irqline);

	//Serial Port TX/RX Callbacks
	// TODO: Move to special port r/w
	void    (*serial_tx_callback)(int data);	//Call back funciton when sending data out of serial port
	int		(*serial_rx_callback)(void);		//Call back function to retrieve data when receiving serial port data

};

/***************************************************************************
    MACROS
***************************************************************************/

#undef change_pc
#define change_pc(x)

/* Read Opcode/Opcode Arguments from Program Code */
#define ROP(pc)			cpu_readop(pc)
#define ROP_ARG(pc)		cpu_readop_arg(pc)

/* Read a byte from External Code Memory (Usually Program Rom(s) Space) */
#define CODEMEM_R(a)	(UINT8)program_read_byte_8le(a)

/* Read/Write a byte from/to External Data Memory (Usually RAM or other I/O) */
#define DATAMEM_R(a)	(UINT8)io_read_byte_8le(a)
#define DATAMEM_W(a,v)	io_write_byte_8le(a,v)
//#define DATAMEM_R(a)  (UINT8)data_read_byte_8le(a)
//#define DATAMEM_W(a,v)    data_write_byte_8le(a,v)

/* Read/Write a byte from/to the Internal RAM */

#define IRAM_R(a)		iram_read(a)
#define IRAM_W(a, d)	iram_write(a, d)

/* Read/Write a byte from/to the Internal RAM indirectly */
/* (called from indirect addressing)                     */
#define IRAM_IR(a) 		data_read_byte_8le((a) & mcs51.ram_mask)
#define IRAM_IW(a, d) 	data_write_byte_8le((a) & mcs51.ram_mask, d)

/* Form an Address to Read/Write to External RAM indirectly */
/* (called from indirect addressing)                        */
#define ERAM_ADDR(a,m)	external_ram_iaddr(a,m)

/* Read/Write a bit from Bit Addressable Memory */
#define BIT_R(a)		bit_address_r(a)
#define BIT_W(a,v)		bit_address_w(a,v)

/* Input/Output a byte from given I/O port */
#define IN(port)		((UINT8)io_read_byte(port))
#define OUT(port,value) io_write_byte(port,value)


/***************************************************************************
    SHORTCUTS
***************************************************************************/

#define PPC 	mcs51.ppc
#define PC		mcs51.pc
#define RWM		mcs51.rwm

/* SFR Registers - These are accessed directly for speed on read */
/* Read accessors                                                */

#define SFR_A(a)		mcs51.sfr_ram[(a)]
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

#define R_REG(r)	mcs51.internal_ram[(r) | (PSW & 0x18)]
#define DPTR		((DPH<<8) | DPL)

/* 8052 Only registers */
#define T2CON		SFR_A(ADDR_T2CON)
#define RCAP2L		SFR_A(ADDR_RCAP2L)
#define RCAP2H		SFR_A(ADDR_RCAP2H)
#define TL2			SFR_A(ADDR_TL2)
#define TH2			SFR_A(ADDR_TH2)

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
#define SET_REG(r, v)	do { mcs51.internal_ram[(r) | (PSW & 0x18)] = (v); } while (0)

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
#define GET_GF1			GET_BIT(TMOD, 3)
#define GET_GF0			GET_BIT(TMOD, 2)
#define GET_PD			GET_BIT(TMOD, 1)		/* Only in 80C51BH */
#define GET_IDL			GET_BIT(TMOD, 0)		/* Only in 80C51BH */

/* 8052 Only flags */
#define GET_TF2			GET_BIT(T2CON, 7)
#define GET_EXF2		GET_BIT(T2CON, 6)
#define GET_RCLK		GET_BIT(T2CON, 5)
#define GET_TCLK		GET_BIT(T2CON, 4)
#define GET_EXEN2		GET_BIT(T2CON, 3)
#define GET_TR2			GET_BIT(T2CON, 2)
#define GET_CT2			GET_BIT(T2CON, 1)
#define GET_CP			GET_BIT(T2CON, 0)

/*Add and Subtract Flag settings*/
#define DO_ADD_FLAGS(a,d,c)	do_add_flags(a,d,c)
#define DO_SUB_FLAGS(a,d,c)	do_sub_flags(a,d,c)

#define SET_PARITY()	do {mcs51.recalc_parity |= 1;} while (0)
#define PUSH_PC()		push_pc()
#define POP_PC()		pop_pc()

#if 0
/* Any pending IRQ */
#define SERIALPORT_IRQ    ((R_SCON & 0x03) && GET_ES)
#define TIMERS_IRQ        ((GET_TF0 && GET_ET0) || (GET_TF1 && GET_ET1))
#define EXTERNAL_IRQ      ((GET_IE0 && GET_EX0) || (GET_IE1 && GET_EX1))

#if (HAS_I8052 || HAS_I8752)
#define NO_PENDING_IRQ  !(TIMERS_IRQ) && !(EXTERNAL_IRQ) && !(SERIALPORT_IRQ) && !(GET_ET2 && (GET_TF2 || GET_EXF2))
#else
#define NO_PENDING_IRQ  !(TIMERS_IRQ) && !(EXTERNAL_IRQ) && !(SERIALPORT_IRQ)
#endif
#endif

/* Clear Current IRQ  */
#define CLEAR_CURRENT_IRQ() do { mcs51.cur_irq = 0xff; mcs51.irq_priority = 0; } while (0)

/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static int mcs51_icount;

static mcs51_regs mcs51;


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void check_irqs(void);
INLINE void serial_transmit(UINT8 data);

/* Hold callback functions so they can be set by caller (before the cpu reset) */

static void (*hold_serial_tx_callback)(int data);
static int (*hold_serial_rx_callback)(void);

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE UINT8 r_acc(void) { return SFR_A(ADDR_ACC); }

INLINE UINT8 r_psw(void) { return SFR_A(ADDR_PSW); }

INLINE void update_ptrs(void)
{
	mcs51.internal_ram = memory_get_write_ptr(cpu_getactivecpu(), ADDRESS_SPACE_DATA, 0x00);
	mcs51.sfr_ram = memory_get_write_ptr(cpu_getactivecpu(), ADDRESS_SPACE_DATA, 0x100);
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

INLINE offs_t external_ram_iaddr(offs_t offset, offs_t mem_mask)
{
	if (mem_mask == 0x00ff)
		return (offset & mem_mask) | (P2 << 8);
#if 0
    if(mcs51.eram_iaddr_callback)
        return mcs51.eram_iaddr_callback(machine,offset,mem_mask);
    else
        if (mem_mask <= 0x100) LOG(("mcs51 #%d: external ram address requested (8 bit offset=%02x), but no callback available! at PC:%04x\n", cpu_getactivecpu(), offset, PC));
#endif
	return offset;
}

/* Internal ram read/write */

INLINE UINT8 iram_read(size_t offset)
{
	return (((offset) < 0x80) ? data_read_byte_8le(offset) : mcs51.sfr_read(offset));
}

INLINE void iram_write(size_t offset, UINT8 data)
{
	if ((offset) < 0x80)
		data_write_byte_8le(offset, data);
	else
		mcs51.sfr_write(offset, data);
}

/*Push the current PC to the stack*/
INLINE void push_pc(void)
{
	UINT8 tmpSP = SP+1;						//Grab and Increment Stack Pointer
	IRAM_IW(tmpSP, (PC & 0xff));				//Store low byte of PC to Internal Ram (Use IRAM_IW to store stack above 128 bytes)
	tmpSP++;									// ""
	SP = tmpSP;								// ""
	IRAM_IW(tmpSP, ( (PC & 0xff00) >> 8));		//Store hi byte of PC to next address in Internal Ram (Use IRAM_IW to store stack above 128 bytes)
}

/*Pop the current PC off the stack and into the pc*/
INLINE void pop_pc(void)
{
	UINT8 tmpSP = SP;							//Grab Stack Pointer
	PC = (IRAM_IR(tmpSP--) & 0xff) << 8;		//Store hi byte to PC (must use IRAM_IR to access stack pointing above 128 bytes)
	PC = PC | IRAM_IR(tmpSP--);					//Store lo byte to PC (must use IRAM_IR to access stack pointing above 128 bytes)
	SP = tmpSP;								//Decrement Stack Pointer
	change_pc(PC);
}

//Set the PSW Parity Flag
INLINE void set_parity(void)
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

INLINE UINT8 bit_address_r(UINT8 offset)
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


INLINE void bit_address_w(UINT8 offset, UINT8 bit)
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

INLINE void do_add_flags(UINT8 a, UINT8 data, UINT8 c)
{
	UINT16 result = a+data+c;
	INT16 result1 = (INT8)a+(INT8)data+c;

	SET_CY((result & 0x100) >> 8);
	result = (a&0x0f)+(data&0x0f)+c;
	SET_AC((result & 0x10) >> 4);
	SET_OV(result1 < -128 || result1 > 127);
}

INLINE void do_sub_flags(UINT8 a, UINT8 data, UINT8 c)
{
	UINT16 result = a-(data+c);
	INT16 result1 = (INT8)a-(INT8)(data+c);

	SET_CY((result & 0x100) >> 8);
	result = (a&0x0f)-((data&0x0f)+c);
	SET_AC((result & 0x10) >> 4);
	SET_OV((result1 < -128 || result1 > 127));
}

INLINE void transmit_receive(int source)
{
	int mode = (GET_SM0<<1) | GET_SM1;

	if (source == 1) /* timer1 */
		mcs51.uart.smod_div = (mcs51.uart.smod_div + 1) & (2-GET_SMOD);

	switch(mode) {
		//8 bit shifter ( + start,stop bit ) - baud set by clock freq / 12
		case 0:
			mcs51.uart.rx_clk += (source == 0) ? 16 : 0; /* clock / 12 */
			mcs51.uart.tx_clk += (source == 0) ? 16 : 0; /* clock / 12 */
			break;
		//8 bit uart ( + start,stop bit ) - baud set by timer1 or timer2
		case 1:
		case 3:
			if (source == 1)
			{
				mcs51.uart.tx_clk += (GET_TCLK ? 0 : !mcs51.uart.smod_div);
				mcs51.uart.rx_clk += (GET_RCLK ? 0 : !mcs51.uart.smod_div);
			}
			if (source == 2)
			{
				mcs51.uart.tx_clk += (GET_TCLK ? 1 : 0);
				mcs51.uart.rx_clk += (GET_RCLK ? 1 : 0);
			}
			break;
		//9 bit uart
		case 2:
			mcs51.uart.rx_clk += (source == 0) ? (GET_SMOD ? 6 : 3) : 0; /* clock / 12 * 3 / 8 (16) = clock / 32 (64)*/
			mcs51.uart.tx_clk += (source == 0) ? (GET_SMOD ? 6 : 3) : 0; /* clock / 12 */
			break;
	}
	/* transmit ? */
	if (mcs51.uart.tx_clk >= 16)
	{
		mcs51.uart.tx_clk &= 0x0f;
		if(mcs51.uart.bits_to_send)
		{
			mcs51.uart.bits_to_send--;
			if(mcs51.uart.bits_to_send == 0) {
				//printf("Here1 %d\n",  mcs51.uart.bits_to_send);
				//Call the callback function
				if(mcs51.serial_tx_callback)
					mcs51.serial_tx_callback(mcs51.uart.data_out);
				//Set Interrupt Flag
				SET_TI(1);
				//Note: we'll let the main loop catch the interrupt
			}
		}

	}
	/* receive */
	if (mcs51.uart.rx_clk >= 16)
	{
		mcs51.uart.rx_clk &= 0x0f;
		if (mcs51.uart.delay_cycles>0)
		{
			mcs51.uart.delay_cycles--;
			if (mcs51.uart.delay_cycles == 0)
			{
				int data = 0;
				//Call our callball function to retrieve the data
				if(mcs51.serial_rx_callback)
					data = mcs51.serial_rx_callback();
				//printf("RX Deliver %d\n", data);
				SET_SBUF(data);
				//Flag the IRQ
				SET_RI(1);
			}
		}
	}
}


INLINE void update_timer_t0(int cycles)
{
	int mode = (GET_M0_1<<1) | GET_M0_0;
	UINT32 count = 0;

	if (GET_TR0)
	{
		UINT32 delta;

		/* counter / external input */
		delta = GET_CT0 ? mcs51.t0_cnt : cycles;
		/* taken, reset */
		mcs51.t0_cnt = 0;
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
				count = (UINT32) TL0 + delta;
				if ( count & 0xffffff00 ) 				/* Check for overflow */
				{
					SET_TF0(1);
					count += TH0; 						/* Reload timer */
				}
				/* Update new values of the counter */
				TL0 =  count & 0xff;
				break;
			case 3:
				/* Split Timer 1 */
				count = (UINT16) TL0 + delta;
				if ( count & 0xffffff00 ) 				/* Check for overflow */
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
			count = (UINT16) TH0 + cycles;			/* No gate control or counting !*/
			if ( count & 0xffffff00 ) 				/* Check for overflow */
				SET_TF1(1);
			TH0 = count & 0xff;						/* Update new values of the counter */
			break;
		}
	}
}

INLINE void update_timer_t1(int cycles)
{
	int mode = (GET_M1_1<<1) | GET_M1_0;
	int mode_0 = (GET_M0_1<<1) | GET_M0_0;
	UINT32 count = 0;

	if (GET_TR1)
	{
		UINT32 delta;
		int overflow = 0;

		/* counter / external input */
		delta = GET_CT1 ? mcs51.t1_cnt : cycles;
		/* taken, reset */
		mcs51.t1_cnt = 0;
		/* TODO: Not sure about IE0. The manual specifies INT0=high,
         * which in turn means CLEAR_LINE. Change to access last_state?
         * IE0 may be edge triggered depending on IT0 */
		if (GET_GATE1 && !GET_IE1)
			delta = 0;

		//printf("mode: %d\n", mode);
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
				count = (UINT32) TL1 + delta;
				overflow = count & 0xffffff00; /* Check for overflow */
				if ( overflow )
				{
					count += TH1; 						/* Reload timer */
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
			/* Timer 1 may still be used in modes 0, 1, and 2, while timer 0 
			 * is in mode 3. With one important exception:  No interrupts 
			 * will be generated by timer 1 while timer 0 is using the TF1 
			 * overflow flag
			 */
			if (mode_0 != 3)
				SET_TF1(1);
			transmit_receive(1);
		}
	}
}

INLINE void update_timer_t2(int cycles)
{
	/* Update Timer 2 */
	if(GET_TR2) {
		int mode = ((GET_TCLK | GET_RCLK) << 1) | GET_CP;
		int delta = GET_CT2 ? mcs51.t2_cnt : (mode & 2) ? cycles * (12/2) : cycles;

		UINT32 count = ((TH2<<8) | TL2) + delta;
		mcs51.t2_cnt = 0;

		switch (mode)
		{
			case 0:	/* 16 Bit Auto Reload */
				if ( count & 0xffff0000 )
				{
					SET_TF2(1);
					count += ((RCAP2H<<8) | RCAP2L);
				}
				else if (GET_EXEN2 && mcs51.t2ex_cnt>0)
				{
					count += ((RCAP2H<<8) | RCAP2L);
					mcs51.t2ex_cnt = 0;
				}
				TH2 = (count>>8) & 0xff;
				TL2 =  count & 0xff;
				break;
			case 1: /* 16 Bit Capture */
				if ( count & 0xffff0000 )
					SET_TF2(1);
				TH2 = (count>>8) & 0xff;
				TL2 =  count & 0xff;

				if (GET_EXEN2 && mcs51.t2ex_cnt>0)
				{
					RCAP2H = TH2;
					RCAP2L = TL2;
					mcs51.t2ex_cnt = 0;
				}
				break;
			case 2:
			case 3:	/* Baud rate */
				if ( count & 0xffff0000 )
				{
					count += ((RCAP2H<<8) | RCAP2L);
					transmit_receive(2);
				}
				TH2 = (count>>8) & 0xff;
				TL2 =  count & 0xff;
				break;
		}
	}
}

INLINE void update_timers(int cycles)
{
	if (cycles == 0)
		return; /* nothing to do */
	/* Update Timer 0 */
	update_timer_t0(cycles);
	update_timer_t1(cycles);

	if (mcs51.features & FEATURE_I8052_UART)
	{
		update_timer_t2(cycles);
	}
}

//Set up to transmit data out of serial port
//NOTE: Enable Serial Port Interrupt bit is NOT required to send/receive data!

INLINE void serial_transmit(UINT8 data)
{
	int mode = (GET_SM0<<1) | GET_SM1;

	//Flag that we're sending data
	mcs51.uart.data_out = data;
	//printf("serial_tansmit: %x %x\n", mode, data);
	switch(mode) {
		//8 bit shifter ( + start,stop bit ) - baud set by clock freq / 12
		case 0:
			mcs51.uart.bits_to_send = 8+2;
			break;
		//8 bit uart ( + start,stop bit ) - baud set by timer1 or timer2
		case 1:
			mcs51.uart.bits_to_send = 8+2;
			break;
		//9 bit uart
		case 2:
		case 3:
			LOG(("Serial mode %d not supported in mcs51!\n", mode));
			break;
	}
}

INLINE void serial_receive(void)
{
	int mode = (GET_SM0<<1) | GET_SM1;

	if(GET_ES && GET_REN) {
		switch(mode) {
			//8 bit shifter ( + start,stop bit ) - baud set by clock freq / 12
			case 0:
				mcs51.uart.delay_cycles = 8+2;
				break;
			//8 bit uart ( + start,stop bit ) - baud set by timer1 or timer2
			case 1:
				mcs51.uart.delay_cycles = 8+2;
				break;
			//9 bit uart
			case 2:
			case 3:
				LOG(("Serial mode %d not supported in mcs51!\n", mode));
				break;
		}
	}
}

//Check and update status of serial port
INLINE void	update_serial(int cycles)
{
	while (--cycles>=0)
		transmit_receive(0);
}

/***************************************************************************
    CALLBACKS - TODO: Remove
***************************************************************************/


void i8051_set_serial_tx_callback(void (*callback)(int data))
{
	//Hold in static variable since this function can get called before reset has run, which wipes i8051 memory clean
	hold_serial_tx_callback = callback;
}

void i8051_set_serial_rx_callback(int (*callback)(void))
{
	//Hold in static variable since this function can get called before reset has run, which wipes i8051 memory clean
	hold_serial_rx_callback = callback;
}

/***************************************************************************
    OPCODES
***************************************************************************/

#include "i8051ops.c"


static void execute_op(UINT8 op)
{
	if (mcs51.recalc_parity)
	{
		set_parity();
		mcs51.recalc_parity = 0;
	}

	switch( op )
	{

		case 0x00:	nop();							break;	//NOP
		case 0x01:	ajmp(op);						break;	//AJMP code addr
		case 0x02:	ljmp();							break;	//LJMP code addr
		case 0x03:	rr_a();							break;	//RR A
		case 0x04:	inc_a();						break;	//INC A
		case 0x05:	RWM=1; inc_mem(); RWM=0;		break;	//INC data addr

		case 0x06:
		case 0x07:	inc_ir(op&1);					break;	//INC @R0/@R1

		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:	inc_r(op&7);					break;	//INC R0 to R7

		case 0x10:	RWM=1; jbc(); RWM=0; 			break; 	//JBC bit addr, code addr
		case 0x11: 	acall(op);						break;	//ACALL code addr
		case 0x12:	lcall(); 						break;	//LCALL code addr
		case 0x13:	rrc_a();						break;	//RRC A
		case 0x14:	dec_a();						break;	//DEC A
		case 0x15:	RWM=1; dec_mem(); RWM=0;		break;	//DEC data addr

		case 0x16:
		case 0x17:	dec_ir(op&1); 					break;	//DEC @R0/@R1

		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:	dec_r(op&7); 					break;	//DEC R0 to R7

		case 0x20:	jb();							break;	//JB  bit addr, code addr
		case 0x21:	ajmp(op);						break;	//AJMP code addr
		case 0x22:	ret();							break;	//RET
		case 0x23:	rl_a();							break;	//RL A
		case 0x24:	add_a_byte();					break;	//ADD A, #data
		case 0x25:	add_a_mem();					break;	//ADD A, data addr

		case 0x26:
		case 0x27:	add_a_ir(op&1);					break;	//ADD A, @R0/@R1

		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:	add_a_r(op&7);					break;	//ADD A, R0 to R7

		case 0x30:	jnb();							break;	//JNB bit addr, code addr
		case 0x31:	acall(op);						break;	//ACALL code addr
		case 0x32:	reti();							break;	//RETI
		case 0x33:	rlc_a();						break;	//RLC A
		case 0x34:	addc_a_byte();					break;	//ADDC A, #data
		case 0x35:	addc_a_mem();					break;	//ADDC A, data addr

		case 0x36:
		case 0x37:  addc_a_ir(op&1);				break;	//ADDC A, @R0/@R1

		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:	addc_a_r(op&7);					break;	//ADDC A, R0 to R7

		case 0x40:	jc();							break;	//JC code addr
		case 0x41:	ajmp(op);						break;	//AJMP code addr
		case 0x42:	RWM=1; orl_mem_a();	RWM=0;		break;	//ORL data addr, A
		case 0x43:	RWM=1; orl_mem_byte(); RWM=0;	break;	//ORL data addr, #data
		case 0x44:	orl_a_byte();					break;
		case 0x45:	orl_a_mem();					break;	//ORL A, data addr

		case 0x46:
		case 0x47:	orl_a_ir(op&1); 				break;	//ORL A, @RO/@R1

		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
		case 0x4c:
		case 0x4d:
		case 0x4e:
		case 0x4f: 	orl_a_r(op&7); 					break;	//ORL A, RO to R7

		case 0x50:	jnc(); 							break;	//JNC code addr
		case 0x51:  acall(op);						break;	//ACALL code addr
		case 0x52:	RWM=1; anl_mem_a(); RWM=0;		break;	//ANL data addr, A
		case 0x53:	RWM=1; anl_mem_byte(); RWM=0;	break;	//ANL data addr, #data
		case 0x54:	anl_a_byte();					break;	//ANL A, #data
		case 0x55:	anl_a_mem();					break;	//ANL A, data addr

		case 0x56:
		case 0x57:	anl_a_ir(op&1);					break;	//ANL A, @RO/@R1

		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
		case 0x5c:
		case 0x5d:
		case 0x5e:
		case 0x5f:	anl_a_r(op&7);					break;	//ANL A, RO to R7

		case 0x60:	jz();							break;	//JZ code addr
		case 0x61:	ajmp(op); 						break;	//AJMP code addr
		case 0x62:	RWM=1; xrl_mem_a(); RWM=0;		break;	//XRL data addr, A
		case 0x63:	RWM=1; xrl_mem_byte(); RWM=0;	break;	//XRL data addr, #data
		case 0x64:	xrl_a_byte();					break;	//XRL A, #data
		case 0x65:	xrl_a_mem();					break;	//XRL A, data addr

		case 0x66:
		case 0x67:	xrl_a_ir(op&1);					break;	//XRL A, @R0/@R1

		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f:	xrl_a_r(op&7);					break;	//XRL A, R0 to R7

		case 0x70:	jnz();							break;	//JNZ code addr
		case 0x71:	acall(op);						break;	//ACALL code addr
		case 0x72:	orl_c_bitaddr();				break;	//ORL C, bit addr
		case 0x73:	jmp_iadptr();					break;	//JMP @A+DPTR
		case 0x74:	mov_a_byte();					break;	//MOV A, #data
		case 0x75:	mov_mem_byte();					break;	//MOV data addr, #data

		case 0x76:
		case 0x77:	mov_ir_byte(op&1);				break;	//MOV @R0/@R1, #data

		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f:	mov_r_byte(op&7);				break;	//MOV R0 to R7, #data

		case 0x80:	sjmp();							break;	//SJMP code addr
		case 0x81:	ajmp(op);						break;	//AJMP code addr
		case 0x82:	anl_c_bitaddr();				break;	//ANL C, bit addr
		case 0x83:	movc_a_iapc();					break;	//MOVC A, @A + PC
		case 0x84:	div_ab();						break;	//DIV AB
		case 0x85:	mov_mem_mem();					break;	//MOV data addr, data addr

		case 0x86:
		case 0x87:	mov_mem_ir(op&1);				break;	//MOV data addr, @R0/@R1

		case 0x88:
		case 0x89:
		case 0x8a:
		case 0x8b:
		case 0x8c:
		case 0x8d:
		case 0x8e:
		case 0x8f:	mov_mem_r(op&7);				break;	//MOV data addr,R0 to R7

		case 0x90:	mov_dptr_byte();				break;	//MOV DPTR, #data
		case 0x91:	acall(op);						break;	//ACALL code addr
		case 0x92:	RWM = 1; mov_bitaddr_c(); RWM = 0; break;	//MOV bit addr, C
		case 0x93:	movc_a_iadptr();				break;	//MOVC A, @A + DPTR
		case 0x94:	subb_a_byte();					break;	//SUBB A, #data
		case 0x95:	subb_a_mem();					break;	//SUBB A, data addr

		case 0x96:
		case 0x97:	subb_a_ir(op&1);				break;	//SUBB A, @R0/@R1

		case 0x98:
		case 0x99:
		case 0x9a:
		case 0x9b:
		case 0x9c:
		case 0x9d:
		case 0x9e:
		case 0x9f:	subb_a_r(op&7);					break;	//SUBB A, R0 to R7

		case 0xa0:	orl_c_nbitaddr();				break;	//ORL C, /bit addr
		case 0xa1:	ajmp(op);						break;	//AJMP code addr
		case 0xa2:	mov_c_bitaddr();				break;	//MOV C, bit addr
		case 0xa3:	inc_dptr();						break;	//INC DPTR
		case 0xa4:	mul_ab();						break;	//MUL AB
		case 0xa5:	illegal(op);					break;	//reserved

		case 0xa6:
		case 0xa7:	mov_ir_mem(op&1);				break;	//MOV @R0/@R1, data addr

		case 0xa8:
		case 0xa9:
		case 0xaa:
		case 0xab:
		case 0xac:
		case 0xad:
		case 0xae:
		case 0xaf:	mov_r_mem(op&7);				break;	//MOV R0 to R7, data addr

		case 0xb0:	anl_c_nbitaddr();				break;	//ANL C,/bit addr
		case 0xb1:	acall(op);						break;	//ACALL code addr
		case 0xb2:	RWM=1; cpl_bitaddr(); RWM=0; 	break;	//CPL bit addr
		case 0xb3:	cpl_c();						break;	//CPL C
		case 0xb4:	cjne_a_byte();					break;	//CJNE A, #data, code addr
		case 0xb5:	cjne_a_mem();					break;	//CJNE A, data addr, code addr

		case 0xb6:
		case 0xb7:	cjne_ir_byte(op&1); 			break;	//CJNE @R0/@R1, #data, code addr

		case 0xb8:
		case 0xb9:
		case 0xba:
		case 0xbb:
		case 0xbc:
		case 0xbd:
		case 0xbe:
		case 0xbf:	cjne_r_byte(op&7); 				break;	//CJNE R0 to R7, #data, code addr

		case 0xc0:	push();							break;	//PUSH data addr
		case 0xc1:	ajmp(op);						break;	//AJMP code addr
		case 0xc2:	RWM=1; clr_bitaddr(); RWM=0;	break;	//CLR bit addr
		case 0xc3:	clr_c(); 						break;	//CLR C
		case 0xc4:	swap_a();						break;	//SWAP A
		case 0xc5:	xch_a_mem();					break;	//XCH A, data addr

		case 0xc6:
		case 0xc7:  xch_a_ir(op&1);					break;	//XCH A, @RO/@R1

		case 0xc8:
		case 0xc9:
		case 0xca:
		case 0xcb:
		case 0xcc:
		case 0xcd:
		case 0xce:
		case 0xcf:	xch_a_r(op&7);					break;	//XCH A, RO to R7

		case 0xd0:	pop();							break;	//POP data addr
		case 0xd1:	acall(op);						break;	//ACALL code addr
		case 0xd2:	RWM=1; setb_bitaddr(); RWM=0;	break;	//SETB bit addr
		case 0xd3:	setb_c();						break;	//SETB C
		case 0xd4:	da_a(); 						break;	//DA A
		case 0xd5:	RWM=1; djnz_mem(); RWM=0;		break;	//DJNZ data addr, code addr

		case 0xd6:
		case 0xd7:  xchd_a_ir(op&1);				break;	//XCHD A, @R0/@R1

		case 0xd8:
		case 0xd9:
		case 0xda:
		case 0xdb:
		case 0xdc:
		case 0xdd:
		case 0xde:
		case 0xdf:	djnz_r(op&7);					break;	//DJNZ R0 to R7,code addr

		case 0xe0:	movx_a_idptr();					break;	//MOVX A,@DPTR
		case 0xe1:	ajmp(op);						break;	//AJMP code addr

		case 0xe2:
		case 0xe3:	movx_a_ir(op&1);				break;	//MOVX A, @R0/@R1

		case 0xe4:	clr_a();						break;	//CLR A
		case 0xe5:	mov_a_mem();					break;	//MOV A, data addr
		case 0xe6:
		case 0xe7:	mov_a_ir(op&1);					break;	//MOV A,@RO/@R1

		case 0xe8:
		case 0xe9:
		case 0xea:
		case 0xeb:
		case 0xec:
		case 0xed:
		case 0xee:
		case 0xef:	mov_a_r(op&7);					break;	//MOV A,R0 to R7

		case 0xf0:	movx_idptr_a();					break;	//MOVX @DPTR,A
		case 0xf1:	acall(op);						break;	//ACALL code addr

		case 0xf2:
		case 0xf3:	movx_ir_a(op&1);				break;	//MOVX @R0/@R1,A

		case 0xf4:	cpl_a();						break;	//CPL A
		case 0xf5:	mov_mem_a();					break;	//MOV data addr, A

		case 0xf6:
		case 0xf7:	mov_ir_a(op&1);					break;	//MOV @R0/@R1, A

		case 0xf8:
		case 0xf9:
		case 0xfa:
		case 0xfb:
		case 0xfc:
		case 0xfd:
		case 0xfe:
		case 0xff:	mov_r_a(op&7);					break;	//MOV R0 to R7, A
		default:
			illegal(op);
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
static void check_irqs(void)
{
	UINT8 ints = (GET_IE0 | (GET_TF0<<1) | (GET_IE1<<2) | (GET_TF1<<3)
			| ((GET_RI|GET_TI)<<4));
	UINT8 ip = IP;
	UINT8 int_vec = 0;
	UINT8 priority_request = 0;
	int i;

	//If All Inerrupts Disabled or no pending abort..
	if(!GET_EA)	return;

	if (mcs51.features & FEATURE_I8052_UART)
		ints |= ((GET_TF2|GET_EXF2)<<5);

	/* mask out interrupts not enabled */
	ints &= IE;

	if (!ints)	return;

	if(mcs51.irq_priority)
	{
		LOG(("high priority irq in progress, skipping irq request\n"));
		return;
	}

	for (i=0; i<mcs51.num_interrupts; i++)
	{
		if (ints & (1<<i))
		{
			int_vec = (i<<3) | 3;
			priority_request = (ip & (1<<i));
			if (priority_request)
				break;
		}
	}

	//Skip the interrupt request if currently processing is lo priority, and the new request IS NOT HI PRIORITY!
	if(mcs51.cur_irq < 0xff && !priority_request)
	{
		LOG(("low priority irq in progress already, skipping low irq request\n"));
		return;
	}

 	//Save current pc to stack, set pc to new interrupt vector
	push_pc();
	PC = int_vec;
	change_pc(PC);

	/* interrupts take 24 cycles */
	mcs51.inst_cycles += 2;

	//Set current Irq & Priority being serviced
	mcs51.cur_irq = int_vec;
	mcs51.irq_priority = priority_request;

	//Clear any interrupt flags that should be cleared since we're servicing the irq!
	switch(mcs51.cur_irq) {
		case V_IE0:
			//External Int Flag only cleared when configured as Edge Triggered..
			if(GET_IT0)  /* for some reason having this, breaks alving dmd games */
				SET_IE0(0);

			/* indicate we took the external IRQ */
			if (mcs51.irq_callback != NULL)
				(*mcs51.irq_callback)(0);

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
			if (mcs51.irq_callback != NULL)
				(*mcs51.irq_callback)(1);

			break;
		case V_TF1:
			//Timer 1 - Always clear Flag
			SET_TF1(0);
			break;
		case V_RITI:
			// no flags are cleared, TI and RI remain set until reset by software
			break;
		/* I8052 specific */
		case V_TF2:
			// no flags are cleared according to manual
			break;
	}
}

INLINE void burn_cycles(int cycles)
{
	/* Update Timer (if any timers are running) */
	update_timers(cycles);

	/* Update Serial (only for mode 0) */
	update_serial(cycles);

	/* check_irqs */
	check_irqs();
}

static void mcs51_set_irq_line(int irqline, int state)
{
	/* From the manual:
     *
     * <cite>In operatiom all the interrupt tlags are latched into the
     * interrupt control system during State 5 of every ma-
     * chine cycle. The samples are polled during the follow-
     * ing machine cycle.</cite>
     *
     * ==> Since we do not emulate substates, this assumes that the signal is present
     * for at least one cycle (12 states)
     *
     */
	int new_state = (mcs51.last_line_state & ~(1 << irqline)) | ((state != CLEAR_LINE) << irqline);
	/* detect 0->1 transistions */
	int tr_state = (~mcs51.last_line_state) & new_state;

	switch( irqline )
	{
		//External Interrupt 0
		case I8051_INT0_LINE:
			//Line Asserted?
			if (state != CLEAR_LINE) {
				//Is the enable flag for this interrupt set?
				if (GET_EX0) {
					//Need cleared->active line transition? (Logical 1-0 Pulse on the line) - CLEAR->ASSERT Transition since INT0 active lo!
					if (GET_IT0) {
						if (GET_BIT(tr_state, I8051_INT0_LINE))
							SET_IE0(1);
					}
					else
						SET_IE0(1);		//Nope, just set it..
				}
			}
			else
			{
				if (!GET_IT0) /* clear if level triggered */
					SET_IE0(0);
			}

			break;

		//External Interrupt 1
		case I8051_INT1_LINE:

			//Line Asserted?
			if (state != CLEAR_LINE) {
				if(GET_EX1) {
					//Need cleared->active line transition? (Logical 1-0 Pulse on the line) - CLEAR->ASSERT Transition since INT1 active lo!
					if(GET_IT1){
						if (GET_BIT(tr_state, I8051_INT1_LINE))
							SET_IE1(1);
					}
					else
						SET_IE1(1);		//Nope, just set it..
				}
			}
			else
			{
				if (!GET_IT1) /* clear if level triggered */
					SET_IE1(0);
			}
			break;

		case MCS51_T0_LINE:
			if (GET_BIT(tr_state, MCS51_T0_LINE))
				mcs51.t0_cnt++;
			break;

		case MCS51_T1_LINE:
			if (GET_BIT(tr_state, MCS51_T1_LINE))
				mcs51.t1_cnt++;
			break;

		case MCS51_T2_LINE:
			if (mcs51.features & FEATURE_I8052_UART)
			{
				if (GET_BIT(tr_state, MCS51_T2_LINE))
					mcs51.t2_cnt++;
			}
			else
				fatalerror("mcs51: Trying to set T2_LINE on a non I8052 type cpu.\n");
			break;

		case MCS51_T2EX_LINE:
			if (mcs51.features & FEATURE_I8052_UART)
			{
				if (GET_TR2 && GET_EXEN2)
					if (GET_BIT(tr_state, MCS51_T2EX_LINE))
					{
						SET_EXF2(1);
						mcs51.t2ex_cnt++;
					}
			}
			else
				fatalerror("mcs51: Trying to set T2EX_LINE on a non I8052 type cpu.\n");
			break;
		//Serial Port Receive
		case I8051_RX_LINE:
			//Is the enable flags for this interrupt set?
			if (state != CLEAR_LINE)
			{
				serial_receive();
			}
			break;
	}
	mcs51.last_line_state = new_state;
}

/* Execute cycles - returns number of cycles actually run */
static int mcs51_execute(int cycles)
{
	UINT8 op;

	mcs51_icount = cycles;

	/* external interrupts may have been set since we last checked */
	mcs51.inst_cycles = 0;
	check_irqs();
	mcs51_icount -= mcs51.inst_cycles;
	burn_cycles(mcs51.inst_cycles);

	do
	{
		/* Read next opcode */
		PPC = PC;
		debugger_instruction_hook(Machine, PC);
		op = cpu_readop(PC++);

		/* process opcode and count cycles */
		mcs51.inst_cycles = mcs51_cycles[op];
		execute_op(op);

		/* burn the cycles */
		mcs51_icount -= mcs51.inst_cycles;
		burn_cycles(mcs51.inst_cycles);

	} while( mcs51_icount > 0 );

	return cycles - mcs51_icount;
}


/****************************************************************************
 * MCS51/8051 Section
 ****************************************************************************/

static void mcs51_sfr_write(size_t offset, UINT8 data)
{
	/* update register */
	assert(offset >= 0x80 && offset <= 0xff);

	/* TODO: Move to memory map */
	switch (offset)
	{
		case ADDR_P0:	OUT(MCS51_PORT_P0,data);	break;
		case ADDR_P1:	OUT(MCS51_PORT_P1,data);	break;
		case ADDR_P2:	OUT(MCS51_PORT_P2,data);	break;
		case ADDR_P3:	OUT(MCS51_PORT_P3,data);	break;
		case ADDR_SBUF:	serial_transmit(data);		break;
		case ADDR_PSW:	SET_PARITY();				break;
		case ADDR_ACC:	SET_PARITY();				break;
		/* R_SBUF = data;        //This register is used only for "Receiving data coming in!" */

		case ADDR_B:
		case ADDR_IP:
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
			LOG(("mcs51 #%d: attemping to write to an invalid/non-implemented SFR address: %zx at 0x%04x, data=%x\n", cpu_getactivecpu(), offset,PC,data));
			/* no write in this case according to manual */
			return;
	}
	data_write_byte_8le((size_t)offset | 0x100, data);
}

static UINT8 mcs51_sfr_read(size_t offset)
{
	assert(offset >= 0x80 && offset <= 0xff);

	switch (offset)
	{
		/* Read/Write/Modify operations read the port latch ! */
		/* Move to memory map */
		case ADDR_P0: 	return RWM ? P0 : P0 & IN(MCS51_PORT_P0);
		case ADDR_P1: 	return RWM ? P1 : P1 & IN(MCS51_PORT_P1);
		case ADDR_P2: 	return RWM ? P2 : P2 & IN(MCS51_PORT_P2);
		case ADDR_P3: 	return RWM ? P3 : P3 & IN(MCS51_PORT_P3);

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
			return data_read_byte_8le((size_t) offset | 0x100);
		/* Illegal or non-implemented sfr */
		default:
			LOG(("mcs51 #%d: attemping to read an invalid/non-implemented SFR address: %zx at 0x%04x\n", cpu_getactivecpu(), offset,PC));
			/* according to the manual, the read may return random bits */
			return 0xff;
	}
}


static void mcs51_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mcs51.irq_callback = irqcallback;

	mcs51.features = FEATURE_NONE;
	mcs51.ram_mask = 0x7F;  			/* 128 bytes of ram */
	mcs51.num_interrupts = 5;			/* 5 interrupts */
	mcs51.sfr_read = mcs51_sfr_read;
	mcs51.sfr_write = mcs51_sfr_write;

	/* Save states */
	state_save_register_item("mcs51", index, mcs51.ppc);
	state_save_register_item("mcs51", index, mcs51.pc);
	state_save_register_item("mcs51", index, mcs51.rwm );
	state_save_register_item("mcs51", index, mcs51.cur_irq );
	state_save_register_item("mcs51", index, mcs51.irq_priority );
	state_save_register_item("mcs51", index, mcs51.last_line_state );
	state_save_register_item("mcs51", index, mcs51.t0_cnt );
	state_save_register_item("mcs51", index, mcs51.t1_cnt );
	state_save_register_item("mcs51", index, mcs51.t2_cnt );
	state_save_register_item("mcs51", index, mcs51.t2ex_cnt );
	state_save_register_item("mcs51", index, mcs51.recalc_parity );
}

static void i80c51_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mcs51_init(index, clock, config, irqcallback);

	mcs51.features |= 	(FEATURE_CMOS_IDLE | FEATURE_CMOS_POWERDOWN);
}

/* Reset registers to the initial values */
static void mcs51_reset(void)
{
	update_ptrs();
	//Set up serial call back handlers
	mcs51.serial_tx_callback = hold_serial_tx_callback;
	hold_serial_tx_callback = NULL;
	mcs51.serial_rx_callback = hold_serial_rx_callback;
	hold_serial_rx_callback = NULL;

	mcs51.last_line_state = 0;
	mcs51.t0_cnt = 0;
	mcs51.t1_cnt = 0;
	mcs51.t2_cnt = 0;
	mcs51.t2ex_cnt = 0;

	/* these are all defined reset states */
	PC = 0;
	SP = 0x7;
	SET_PSW(0);
	SET_ACC(0);
	DPH = 0;
	DPL = 0;
	B = 0;
	IP = 0;
	IE = 0;
	SCON = 0;
	TCON = 0;
	TMOD = 0;
	TH1 = 0;
	TH0 = 0;
	TL1 = 0;
	TL0 = 0;
	/* set the port configurations to all 1's */
	SET_P3(0xff);
	SET_P2(0xff);
	SET_P1(0xff);
	SET_P0(0xff);

	/* Flag as NO IRQ in Progress */
	CLEAR_CURRENT_IRQ();
}

/* Shut down CPU core */
static void mcs51_exit(void)
{
	/* nothing to do */
}

/****************************************************************************
 * 8052 Section
 ****************************************************************************/

static void i8052_sfr_write(size_t offset, UINT8 data)
{
	switch (offset)
	{
		/* 8052 family specific */
		case ADDR_T2CON:
		case ADDR_RCAP2L:
		case ADDR_RCAP2H:
		case ADDR_TL2:
		case ADDR_TH2:
			data_write_byte_8le((size_t) offset | 0x100, data);
			break;

		default:
			mcs51_sfr_write(offset, data);
	}
}

static UINT8 i8052_sfr_read(size_t offset)
{
	switch (offset)
	{
		/* 8052 family specific */
		case ADDR_T2CON:
		case ADDR_RCAP2L:
		case ADDR_RCAP2H:
		case ADDR_TL2:
		case ADDR_TH2:
			return data_read_byte_8le((size_t) offset | 0x100);
		default:
			return mcs51_sfr_read(offset);
	}
}

static void i8052_init (int index, int clock, const void *config, int (*irqcallback)(int))
{
	mcs51_init(index, clock, config, irqcallback);

	mcs51.ram_mask = 0xFF;  			/* 256 bytes of ram */
	mcs51.num_interrupts = 6;			/* 6 interrupts */

	mcs51.features |= FEATURE_I8052_UART;
	mcs51.sfr_read = i8052_sfr_read;
	mcs51.sfr_write = i8052_sfr_write;
}

static void i80c52_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	i8052_init(index, clock, config, irqcallback);

	mcs51.features |= (FEATURE_CMOS_IDLE | FEATURE_CMOS_POWERDOWN);
}

static void i8052_reset(void)
{
	mcs51_reset();

	//8052 Only registers
	T2CON = 0;
	RCAP2L = 0;
	RCAP2H = 0;
	TL2 = 0;
	TH2 = 0;
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/


/* FIXME: the memory maps should probably support rom banking for EA */
static ADDRESS_MAP_START(program_12bit, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x00, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(program_13bit, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x00, 0x1fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_7bit, ADDRESS_SPACE_DATA, 8)
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x0100, 0x01ff) AM_RAM /* SFR */
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_8bit, ADDRESS_SPACE_DATA, 8)
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x01ff) AM_RAM /* SFR */
ADDRESS_MAP_END


/***************************************************************************
    GENERAL CONTEXT ACCESS
***************************************************************************/

static void mcs51_get_context(void *dst)
{
	if( dst )
		memcpy(dst, &mcs51, sizeof(mcs51_regs));
}

static void mcs51_set_context(void *src)
{
	if( src )
		memcpy(&mcs51, src, sizeof(mcs51_regs));
		change_pc(PC);
		update_ptrs();
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void mcs51_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:							PC = info->i;							break;
		case CPUINFO_INT_SP:							SP = info->i;		     				break;

		case CPUINFO_INT_INPUT_STATE + I8051_INT0_LINE:	mcs51_set_irq_line(I8051_INT0_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + I8051_INT1_LINE:	mcs51_set_irq_line(I8051_INT1_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + MCS51_T0_LINE:	mcs51_set_irq_line(MCS51_T0_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + MCS51_T1_LINE:	mcs51_set_irq_line(MCS51_T1_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + I8051_RX_LINE:	mcs51_set_irq_line(I8051_RX_LINE, info->i); break;

		case CPUINFO_INT_REGISTER + I8051_PC: 			PC = info->i;								break;
		case CPUINFO_INT_REGISTER + I8051_SP: 			SP = info->i;							break;
		case CPUINFO_INT_REGISTER + I8051_PSW:			SET_PSW(info->i);							break;
		case CPUINFO_INT_REGISTER + I8051_ACC:			SET_ACC(info->i);							break;
		case CPUINFO_INT_REGISTER + I8051_B:  			B = info->i;								break;
		case CPUINFO_INT_REGISTER + I8051_DPH:			DPH = info->i;							break;
		case CPUINFO_INT_REGISTER + I8051_DPL:			DPL = info->i;							break;
		case CPUINFO_INT_REGISTER + I8051_IE: 			IE = info->i;							break;
		case CPUINFO_INT_REGISTER + I8051_R0: 			SET_REG(0, info->i); break;
		case CPUINFO_INT_REGISTER + I8051_R1: 			SET_REG(1, info->i); break;
		case CPUINFO_INT_REGISTER + I8051_R2: 			SET_REG(2, info->i); break;
		case CPUINFO_INT_REGISTER + I8051_R3: 			SET_REG(3, info->i); break;
		case CPUINFO_INT_REGISTER + I8051_R4: 			SET_REG(4, info->i); break;
		case CPUINFO_INT_REGISTER + I8051_R5: 			SET_REG(5, info->i); break;
		case CPUINFO_INT_REGISTER + I8051_R6: 			SET_REG(6, info->i); break;
		case CPUINFO_INT_REGISTER + I8051_R7: 			SET_REG(7, info->i); break;
		case CPUINFO_INT_REGISTER + I8051_RB: 			SET_RS(info->i); break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void mcs51_get_info(UINT32 state, cpuinfo *info)
{
	mcs51_regs *r = &mcs51;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(mcs51);				break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 12;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 5;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 20; /* rough guess */			break;
		case CPUINFO_INT_INPUT_LINES:        			info->i = 3;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 9; /* due to sfr mapping */					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:	info->i = 8;						break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 	info->i = 17;						break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 	info->i = 0;						break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = PPC;							break;
		case CPUINFO_INT_PC:	 						info->i = PC;							break;
		case CPUINFO_INT_SP:							info->i = SP;							break;

		case CPUINFO_INT_REGISTER + I8051_PC: 			info->i = PC;							break;
		case CPUINFO_INT_REGISTER + I8051_SP: 			info->i = SP;							break;
		case CPUINFO_INT_REGISTER + I8051_PSW:			info->i = PSW;							break;
		case CPUINFO_INT_REGISTER + I8051_ACC:			info->i = ACC;							break;
		case CPUINFO_INT_REGISTER + I8051_B:  			info->i = B;							break;
		case CPUINFO_INT_REGISTER + I8051_DPH:			info->i = DPH;							break;
		case CPUINFO_INT_REGISTER + I8051_DPL:			info->i = DPL;							break;
		case CPUINFO_INT_REGISTER + I8051_IE: 			info->i = IE;							break;
		case CPUINFO_INT_REGISTER + I8051_R0: 			info->i = R_REG(0);	 					break;
		case CPUINFO_INT_REGISTER + I8051_R1: 			info->i = R_REG(1); 					break;
		case CPUINFO_INT_REGISTER + I8051_R2: 			info->i = R_REG(2); 					break;
		case CPUINFO_INT_REGISTER + I8051_R3: 			info->i = R_REG(3); 					break;
		case CPUINFO_INT_REGISTER + I8051_R4: 			info->i = R_REG(4); 					break;
		case CPUINFO_INT_REGISTER + I8051_R5: 			info->i = R_REG(5); 					break;
		case CPUINFO_INT_REGISTER + I8051_R6: 			info->i = R_REG(6); 					break;
		case CPUINFO_INT_REGISTER + I8051_R7: 			info->i = R_REG(7); 					break;
		case CPUINFO_INT_REGISTER + I8051_RB: 			info->i = R_REG(8); 					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = mcs51_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = mcs51_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = mcs51_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = mcs51_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = mcs51_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = mcs51_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = mcs51_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = i8051_dasm;			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &mcs51_icount;			break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map8 = NULL;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:    info->internal_map8 = NULL;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_IO:      info->internal_map8 = NULL;	break;

		case CPUINFO_STR_NAME:							strcpy(info->s, "I8051");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "MCS-51");				break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Steve Ellenoff"); break;

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

		case CPUINFO_STR_REGISTER + I8051_PC:        	sprintf(info->s, "PC:%04X", r->pc);		break;
		case CPUINFO_STR_REGISTER + I8051_SP:        	sprintf(info->s, "SP:%02X", SP);		break;
		case CPUINFO_STR_REGISTER + I8051_PSW:       	sprintf(info->s, "PSW:%02X", PSW);		break;
		case CPUINFO_STR_REGISTER + I8051_ACC:       	sprintf(info->s, "A:%02X", ACC);		break;
		case CPUINFO_STR_REGISTER + I8051_B:         	sprintf(info->s, "B:%02X", B);			break;
		case CPUINFO_STR_REGISTER + I8051_DPH:       	sprintf(info->s, "DPH:%02X", DPH);		break;
		case CPUINFO_STR_REGISTER + I8051_DPL:       	sprintf(info->s, "DPL:%02X", DPL);		break;
		case CPUINFO_STR_REGISTER + I8051_IE:        	sprintf(info->s, "IE:%02X", IE);		break;
		case CPUINFO_STR_REGISTER + I8051_R0:        	sprintf(info->s, "R0:%02X", R_REG(0)); 	break;
		case CPUINFO_STR_REGISTER + I8051_R1:        	sprintf(info->s, "R1:%02X", R_REG(1)); 	break;
		case CPUINFO_STR_REGISTER + I8051_R2:        	sprintf(info->s, "R2:%02X", R_REG(2)); 	break;
		case CPUINFO_STR_REGISTER + I8051_R3:			sprintf(info->s, "R3:%02X", R_REG(3)); 	break;
		case CPUINFO_STR_REGISTER + I8051_R4: 			sprintf(info->s, "R4:%02X", R_REG(4)); 	break;
		case CPUINFO_STR_REGISTER + I8051_R5: 			sprintf(info->s, "R5:%02X", R_REG(5)); 	break;
		case CPUINFO_STR_REGISTER + I8051_R6: 			sprintf(info->s, "R6:%02X", R_REG(6)); 	break;
		case CPUINFO_STR_REGISTER + I8051_R7: 			sprintf(info->s, "R7:%02X", R_REG(7)); 	break;
		case CPUINFO_STR_REGISTER + I8051_RB: 			sprintf(info->s, "RB:%02X", ((PSW & 0x18)>>3)); break;
	}
}

/**************************************************************************
 * Specific get_info
 **************************************************************************/

void i8031_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:    info->internal_map8 = address_map_data_7bit;	break;

		case CPUINFO_STR_NAME:							strcpy(info->s, "I8031");				break;
		default:										mcs51_get_info(state, info);			break;
	}
	/* --- the following bits of info are returned as NULL-terminated strings --- */
}

void i8051_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map8 = address_map_program_12bit;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:    info->internal_map8 = address_map_data_7bit;	break;

		case CPUINFO_STR_NAME:							strcpy(info->s, "I8051");				break;
		default:										mcs51_get_info(state, info);			break;
	}
	/* --- the following bits of info are returned as NULL-terminated strings --- */
}

void i8032_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_PTR_INIT:							info->init = i8052_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = i8052_reset;				break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:    info->internal_map8 = address_map_data_8bit;	break;

		case CPUINFO_STR_NAME:							strcpy(info->s, "I8032");				break;

		default:										mcs51_get_info(state, info);			break;
	}
}

void i8052_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_PTR_INIT:							info->init = i8052_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = i8052_reset;				break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map8 = address_map_program_13bit;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:    info->internal_map8 = address_map_data_8bit;	break;

		case CPUINFO_STR_NAME:							strcpy(info->s, "I8052");				break;

		default:										mcs51_get_info(state, info);			break;
	}
}

void i8751_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map8 = address_map_program_12bit;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:    info->internal_map8 = address_map_data_7bit;	break;

		case CPUINFO_STR_NAME:							strcpy(info->s, "I8751");				break;

		default:										mcs51_get_info(state, info);			break;
	}
}

void i8752_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_PTR_INIT:							info->init = i8052_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = i8052_reset;				break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map8 = address_map_program_13bit;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:    info->internal_map8 = address_map_data_8bit;	break;

		case CPUINFO_STR_NAME:							strcpy(info->s, "I8752");				break;

		default:										mcs51_get_info(state, info);			break;
	}
}

/**************************************************************************
 * CMOS get_info
 **************************************************************************/

void i80c31_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_PTR_INIT:							info->init = i80c51_init;				break;
		case CPUINFO_STR_NAME:							strcpy(info->s, "I80C31");				break;
		default:										i8031_get_info(state, info);			break;
	}
}

void i80c51_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_PTR_INIT:							info->init = i80c51_init;				break;
		case CPUINFO_STR_NAME:							strcpy(info->s, "I80C51");				break;
		default:										i8051_get_info(state, info);			break;
	}
}

void i80c32_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_PTR_INIT:							info->init = i80c52_init;				break;
		case CPUINFO_STR_NAME:							strcpy(info->s, "I80C32");				break;
		default:										i8032_get_info(state, info);			break;
	}
}

void i80c52_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_PTR_INIT:							info->init = i80c52_init;				break;
		case CPUINFO_STR_NAME:							strcpy(info->s, "I80C52");				break;
		default:										i8052_get_info(state, info);			break;
	}
}

void i87c51_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_PTR_INIT:							info->init = i80c51_init;				break;
		case CPUINFO_STR_NAME:							strcpy(info->s, "I87C51");				break;
		default:										i8751_get_info(state, info);			break;
	}
}

void i87c52_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_PTR_INIT:							info->init = i80c52_init;				break;
		case CPUINFO_STR_NAME:							strcpy(info->s, "I87C52");				break;
		default:										i8752_get_info(state, info);			break;
	}
}

/**************************************************************************
 * Other variants get_info
 **************************************************************************/

void at89c4051_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_PTR_INIT:							info->init = i80c51_init;				break;
		case CPUINFO_STR_NAME:							strcpy(info->s, "AT89C4051");				break;
		default:										i8051_get_info(state, info);			break;
	}
}
